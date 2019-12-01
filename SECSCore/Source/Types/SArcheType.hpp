#pragma once
#include "SEntity.hpp"
#include "SComponent.hpp"
#include <cassert>
#include <iostream>
#include "Containers\SChunkList.hpp"
#include "..\TemplateUtils\TemplatePackUtils.hpp"


namespace SECS
{
	struct SChunk;
	// ArcheType.
	struct SArcheType
	{
		friend class SEntityManager;
		friend class SArcheTypeManager;
		friend struct SChunk;
		friend struct ComponentSystemData;
		friend class SSystem;
	protected:
		int ComponentNum = 0;
		size_t EntitySize = 0;
		size_t ComponentTotalSize = 0;
		// Insert sorted at init time.
		size_t* typeHashes;
		// Offsets in the chunk.
		size_t* ComponentOffsets;
		size_t* SizeOfs;
		SChunkList* chunks = nullptr;
		SChunk* freeChunk = nullptr;
	public:
		SArcheType(size_t _EntitySize, size_t _ComponentTotalSize, size_t _ComponentNum)
			:EntitySize(_EntitySize), ComponentTotalSize(_ComponentTotalSize), ComponentNum(_ComponentNum)
		{
			chunks = nullptr;
			freeChunk = nullptr;
			typeHashes = new size_t[ComponentNum]();
			SizeOfs = new size_t[ComponentNum]();
			ComponentOffsets = new size_t[ComponentNum]();
		}
		SArcheType(SArcheType&& arc) = default;

		template<typename ... Ts>
		inline SArcheType Shrink()
		{
			size_t* _hashes = TemplatePackUtils::GetHashes_Torder<Ts...>();
			size_t* _sizes = TemplatePackUtils::TemplatePack_TSizes<Ts...>();
			return Shrink(_hashes, sizeof...(Ts), _sizes);
		}

		template<typename ... Ts>
		inline SArcheType Expand() noexcept
		{
			size_t* _hashes = TemplatePackUtils::GetHashes_Torder<Ts...>();
			size_t* _sizes = TemplatePackUtils::TemplatePack_TSizes<Ts...>();
			return Expand(_hashes, sizeof...(Ts), _sizes);
		}

		inline SArcheType Shrink(size_t* _InHash, int _HashLength, size_t* _ComponentSizes)
		{
			assert(_HashLength > 0);
			SArcheType _newArche = SArcheType(EntitySize, 0, ComponentNum - _HashLength);
			size_t src_I = 0;
			size_t dst_I = 0;
			// 
			while (dst_I < ComponentNum)
			{
				auto st = _InHash[src_I];
				auto dt = typeHashes[dst_I];
				if (src_I < _HashLength)
				{
					if (st != dt)
					{
						_newArche.typeHashes[dst_I] = typeHashes[dst_I];
						_newArche.SizeOfs[dst_I] = SizeOfs[dst_I];
						_newArche.ComponentTotalSize += SizeOfs[dst_I];
						dst_I++;
					}
					else { src_I++; }
				}
				else if (src_I == _HashLength)
				{
					// Copy the rest
					_newArche.typeHashes[dst_I] = typeHashes[dst_I];
					_newArche.SizeOfs[dst_I] = SizeOfs[dst_I];
					_newArche.ComponentTotalSize += SizeOfs[dst_I];
					dst_I++;
				}
			}
			// Recompute offsets
			_newArche.ComponentOffsets[0] = 0;
			for (dst_I = 1; dst_I < _newArche.ComponentNum; dst_I++)
			{
				_newArche.ComponentOffsets[dst_I] = _newArche.ComponentOffsets[dst_I - 1] + _newArche.SizeOfs[dst_I - 1];
			}
			return _newArche;
		}

		inline SArcheType Expand(size_t* _InHash, int _HashLength, size_t* _ComponentSizes) noexcept
		{
			assert(_HashLength > 0);
			SArcheType _newArche = SArcheType(EntitySize, ComponentTotalSize, ComponentNum + _HashLength);
			memcpy(_newArche.typeHashes, typeHashes, ComponentNum * sizeof(size_t));
			memcpy(_newArche.SizeOfs, SizeOfs, ComponentNum * sizeof(size_t));
			// insert 
			_HashLength--;
			int i = _newArche.ComponentNum - _HashLength - 2;
			for (_HashLength; _HashLength >= 0; _HashLength--)
			{
				for (i = _newArche.ComponentNum - _HashLength - 2; i >= 0 && _newArche.typeHashes[i] > _InHash[_HashLength]; i--)
				{
					_newArche.typeHashes[i + 1] = _newArche.typeHashes[i];
					_newArche.SizeOfs[i + 1] = _newArche.SizeOfs[i];
				}
				_newArche.typeHashes[i + 1] = _InHash[_HashLength];
				_newArche.SizeOfs[i + 1] = _ComponentSizes[_HashLength];
				// Insert finish, comp num++
				_newArche.ComponentTotalSize += _ComponentSizes[_HashLength];
			}
			// Recompute offsets
			_newArche.ComponentOffsets[0] = 0;
			for (i = 1; i < _newArche.ComponentNum; i++)
			{
				_newArche.ComponentOffsets[i] = _newArche.ComponentOffsets[i - 1] + _newArche.SizeOfs[i - 1];
			}
			// return moved
			return _newArche;
		}
	public:
		inline bool Is(size_t _num, size_t* _typeHashes)
		{
			if (_num != this->ComponentNum) return false;
			while (_num > 0)
			{
				_num--;
				if (typeHashes[_num] != _typeHashes[_num]) return false;
			}
			return true;
		}
		inline bool Is(const SArcheType* _arc)
		{
			return Is(_arc->ComponentNum, _arc->typeHashes);
		}
		inline bool Includes(size_t _num, size_t* _typehashes)
		{
			if (_num > ComponentNum || typeHashes[0] > _typehashes[0])
				return false;
			size_t FocusHash = 0;
			for (int i = 0; i < ComponentNum; i++)
			{
				if (typeHashes[i] == _typehashes[FocusHash]) FocusHash += 1;
				// All match, return true
				if (FocusHash == _num)
					return true;
				if(typeHashes[i] > _typehashes[_num - 1])
					return false;
			}
			return false;
		}
		inline bool Includes(SArcheType* _arc)
		{
			return Includes(_arc->ComponentNum, _arc->typeHashes);
		}

		inline bool Included(size_t _num, size_t* _typehashes)
		{
			if (ComponentNum > _num || typeHashes[0] < _typehashes[0])
				return false;
			size_t FocusHash = 0;
			for (int i = 0; i < _num; i++)
			{
				if (_typehashes[i] == typeHashes[FocusHash]) FocusHash += 1;
				// All match, return true
				if (FocusHash == ComponentNum)
					return true;
				if (_typehashes[i] > typeHashes[_num - 1])
					return false;
			}
			return false;
		}

		inline bool Included(SArcheType* _arc)
		{
			return _arc->Includes(this);
		}
	public:
		// Returns -1 when not such component found.
		template<typename T>
		inline int GetComponentIndex()
		{
			size_t _hash = typeid(T).hash_code();
			for (size_t i = 0; i < ComponentNum; i++)
			{
#if defined(DEBUG_ARCHETYPE)
				std::cout << std::endl << "ArcheType: Get Comp Index Output component hash " << typeHashes[i] << std::endl;
#endif
				if (_hash == typeHashes[i])
					return i;
			}
			return -1;
		}
		// hash code version of GetComponentIndex<T>()
		inline int GetComponentIndex(size_t _hash)
		{
			for (size_t i = 0; i < ComponentNum; i++)
			{
				if (_hash == typeHashes[i]) return i;
			}
			return -1;
		}
	protected:
		template<typename __C>
		inline void __init__Internal()
		{
			size_t _hash = typeid(__C).hash_code();
			int i = ComponentNum - 1;
			for (i; i >= 0 & _hash < typeHashes[i]; i--)
			{
				typeHashes[i + 1] = typeHashes[i];
				SizeOfs[i+1] = SizeOfs[i];
			}
			typeHashes[i + 1] = _hash;
			SizeOfs[i+1] = sizeof(__C);

			ComponentNum += 1;
			ComponentTotalSize += sizeof(__C);
#if defined(DEBUG) || defined(_DEBUG)
			std::cout << "Current " << ComponentNum << " Objects in SArcheType " << "   " << typeid(__C).hash_code();
			std::cout << typeid(__C).name() << ": \nSize: " << SizeOfs[i + 1] << ", Offset: " << ComponentOffsets[i + 1];
			std::cout << ", Total Size: " << ComponentTotalSize << std::endl;
#endif
		}
		template<typename __C1, typename __C2, typename ... __Cs>
		inline void __init__Internal()
		{
			__init__Internal<__C1>();
			__init__Internal<__C2, __Cs...>();
		}

		template<typename ... Components>
		inline void Init(SEntity* e)
		{
#if defined(DEBUG) || defined(_DEBUG)
			std::cout << std::endl << "Start construct: " << typeid(*this).name() << std::endl;
#endif
			typeHashes = new size_t[sizeof...(Components)]();
			ComponentOffsets = new size_t[sizeof...(Components)]();
			SizeOfs = new size_t[sizeof...(Components)]();
			EntitySize = sizeof(*e);
			__init__Internal<Components...>();

			ComponentOffsets[0] = 0;
			for (size_t i = 1; i < ComponentNum; i++)
			{
				ComponentOffsets[i] = ComponentOffsets[i - 1] + SizeOfs[i - 1];
			}
#if defined(DEBUG) || defined(_DEBUG) 
			std::cout << "End construct! " << typeid(*this).name() << std::endl << std::endl;
#endif
		}
		SArcheType()
		{

		}
		// Not implemented.
		template<typename __C>
		inline __C* __findInChunk(SChunk* __chunk)
		{
			return nullptr;
		}
		// Not implemented.
		template<typename __C, typename ... Params>
		inline void __createComponentAtWithParams(Params&& ... __params)
		{
			__C* __ptr = __findInChunk<__C>();
			new(__ptr) __C(__params...);
		}
	public:
		friend std::ostream& operator <<(std::ostream& out, const SArcheType& ar)
		{
			out << "ArcheType Component Num: " << ar.ComponentNum << std::endl;
			for (int i = 0; i < ar.ComponentNum; i++)
				out << "ArcheType Component SizeOfs: " << *(ar.SizeOfs + i) << std::endl;
			out << "ArcheType Component Total Size: " << ar.ComponentTotalSize << std::endl;
			for (int i = 0; i < ar.ComponentNum; i++)
				out << "ArcheType Component typeHash: " << *(ar.typeHashes + i) << "  " << std::endl;
			for (int i = 0; i < ar.ComponentNum; i++)
				out << "ArcheType Component ComponentOffsets: " << *(ar.ComponentOffsets + i) << std::endl;
			return out;
		}
	};
}
	