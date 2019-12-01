/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:					Chunk master class.
Details:		
*******************************************************************************************/
#pragma once
#include <stdlib.h>
#include "SArcheType.hpp"
#include <corecrt_malloc.h>

namespace SECS
{
	struct SArcheType;
	class SArcheTypeManager;
	class SEntityManager;
	///<summary>
	/// Chunk is a block buffer that contains properties
	/// and components of archetype.
	///</summary>
	struct SChunk
	{
		friend class SArcheTypeManager;
		friend struct SArcheType;
		friend struct SEntity;
		friend class SEntityManager;
	protected:
		// aligned structure of size 256
		struct ChunkProperties
		{
			alignas(8) SArcheType* ArcheType = nullptr;
			// Count of units in this chunk.
			alignas(8) int32_t Count = -1;
			// Index of this in the ChunkList.
			alignas(8) int32_t ListIndex = -1;

			alignas(8) size_t ChunkBuffSize = 16 * 1024 - 256;

			alignas(8) size_t FreeUnits = 0;
			// Pad
			alignas(1) char Flag[256 - 40] = {0};
		};
		// Size 16*1024
		void* buff = nullptr;

		void* entityPtr = nullptr;
		void** comListPtrs = nullptr;

		void** wrkptrs = nullptr;
		void* entityWrkPtr = nullptr;
		ChunkProperties* properties = nullptr;
	public:
		ChunkProperties* GetChunkProperty()
		{
			return properties;
		}
		~SChunk()
		{
			free(buff);
		}
	protected:
		SChunk()
		{
			buff = malloc(16 * 1024);
			unsigned char* ptr = (unsigned char*)buff;
			properties = new(ptr)ChunkProperties();
			// Rest: Chunk data that contains components and entities.
			// Cause we need to know exactly how the layout is
			// we need to have a template version call.
		}
		
		inline size_t GetProperUnitCount(size_t chunkSize, size_t arcSize) noexcept
		{
			return chunkSize / arcSize;
		}

		// Returns work ptr for type T.
		template<typename T>
		inline void* __getCompWrkPtr() noexcept
		{
			int index = properties->ArcheType->GetComponentIndex<T>();
			if (index >= 0)
			{
				return wrkptrs[index];
			}
			return nullptr;
		}

		// get entity ptr.
		template<typename T>
		inline T* __getEntityPtr(size_t n) noexcept
		{
			if (n > properties->Count - properties->FreeUnits)
			{
				return nullptr;
			}
			return (T*)entityPtr + n;
		}

		// get entity ptr.
		inline SEntity* __getEntityPtr(size_t n) noexcept
		{		
			return (SEntity*)((unsigned char*)buff + sizeof(ChunkProperties) + n * sizeof(SEntity));
		}

		// get comp ptr.
		template<typename T>
		inline T* __getCompPtr(size_t n) noexcept
		{	
			return ((T*)(comListPtrs[properties->ArcheType->GetComponentIndex<T>()])) + n;
		}

		// get comp ptr with num and index.
		// extremely unsafe, no class data about the component would be return.
		inline void* __unsafeGetCompPtr(size_t componentIndex, size_t unitIndex) noexcept
		{
			// byte offset
			return (unsigned char*)comListPtrs[componentIndex] + unitIndex * properties->ArcheType->SizeOfs[componentIndex];
		}

		// use free to avoid ~Component call.
		inline void __clearEntityComponentLast() noexcept
		{
			properties->FreeUnits += 1;
		}

		inline SEntity* __moveLastEntityComponentFrom(SChunk* _src, size_t _indexTo) noexcept
		{
			return __moveEntityComponentFrom(_src, _indexTo, _src->properties->Count - _src->properties->FreeUnits - 1);
		}

		// unsafe, will cover current entity component instance.
		// operate on chunks with different archetypes will return false.
		inline SEntity* __moveEntityComponentFrom(SChunk* _src, size_t _indexTo, size_t _indexFrom) noexcept
		{
			// precheck
			ChunkProperties* chunkProp = properties;
			if (_src->properties == properties && _indexTo == _indexFrom)
			{
				properties->FreeUnits -= 1;
				return __getEntityPtr(_indexTo);
			}	
			if ((_indexTo > this->properties->Count - 1) || (_indexFrom > _src->properties->Count - 1))
				return nullptr;
			// start loop component move
			uint16_t dstI = 0;
			uint16_t srcI = 0;
			SArcheType* dstType = properties->ArcheType;
			SArcheType* srcType = _src->properties->ArcheType;
			memcpy((unsigned char*)__getEntityPtr(_indexTo), (unsigned char*)_src->__getEntityPtr(_indexFrom), dstType->EntitySize);
			while (srcI < srcType->ComponentNum && dstI < dstType->ComponentNum)
			{
				auto st = srcType->typeHashes[srcI];
				auto dt = dstType->typeHashes[dstI];
				unsigned char* s = (unsigned char*)_src->__unsafeGetCompPtr(srcI, _indexFrom);
				unsigned char* d = (unsigned char*)__unsafeGetCompPtr(dstI, _indexTo);
				if (st < dt) srcI++; // delete source comp(do nothing)
				else if (st > dt) //construct(set 0)
					memset(d, 0, dstType->SizeOfs[dstI++]);
				else //move
					memcpy(d, s, dstType->SizeOfs[(srcI++, dstI++)]);
			}
			//properties->FreeUnits -= 1;
			// Toggle usable
			_src->__clearEntityComponentLast();
			SEntity* ent = __getEntityPtr(_indexTo);

			return __getEntityPtr(_indexTo);
		}

		// Returns false when the chunk is full.
		inline bool __offsetWrkptrs(size_t units) noexcept
		{
			properties->FreeUnits -= units;
			if (properties->FreeUnits <= 0) return false;
			entityWrkPtr = (unsigned char*)entityWrkPtr + properties->ArcheType->EntitySize * units;
			for (size_t i = 0; i < properties->ArcheType->ComponentNum; i++)
			{
				// Do offset works.
				wrkptrs[i] = (unsigned char*)wrkptrs[i] + properties->ArcheType->SizeOfs[i] * units;
			}
			return true;
		}

		// Init Chunk Layout with given Component templates.
		inline void InitChunkLayout(SArcheType* arcType) noexcept
		{
			properties->ArcheType = arcType;
			// Calculate proper unit of the hole chunk
			properties->Count = GetProperUnitCount(properties->ChunkBuffSize, arcType->ComponentTotalSize + arcType->EntitySize);
			properties->FreeUnits = properties->Count;
#if defined(DEBUG) || defined(_DEBUG)
			std::cout << "SChunk: Generate " << properties->Count << " Units" << std::endl;
#endif
			wrkptrs = new void* [arcType->ComponentNum];
			comListPtrs = new void* [arcType->ComponentNum];
			entityPtr = (unsigned char*)buff + sizeof(ChunkProperties);
			entityWrkPtr = entityPtr;
			comListPtrs[0] = (unsigned char*)entityWrkPtr + properties->Count * arcType->EntitySize;
			unsigned char* startptr = (unsigned char*)comListPtrs[0];
			for (size_t i = 0; i < arcType->ComponentNum; i++)
			{
				// Cache pointer for every component.
				comListPtrs[i] = (unsigned char*)startptr + properties->Count * (arcType->ComponentOffsets[i]);
				wrkptrs[i] = comListPtrs[i];
#if defined(DEBUG) || defined(_DEBUG)
				std::cout << "SChunk: malloc," << properties->ArcheType->typeHashes[i] << " ptr offset: " << (unsigned char*)wrkptrs[i] - (unsigned char*)buff << std::endl;
#endif
			}
			// properties->ChunkBuffSize - arcType->ComponentTotalSize * properties->Count
			//memset(startptr + arcType->ComponentTotalSize * properties->Count, 0, properties->ChunkBuffSize - (arcType->ComponentTotalSize + arcType->EntitySize) * properties->Count);
#if defined(DEBUG) || defined(_DEBUG)
			std::cout << std::endl;
#endif
			// Init finish.
		}

		template<typename T>
		inline T* __constructOnChunk_Internal() noexcept
		{
			T* Tptr = (T*)__getCompWrkPtr<T>();
#if defined(DEBUG) || defined(_DEBUG)
			std::cout << "Sakura Chunk: Construct Comp " << typeid(T).name() << " offset to buffer: " << (unsigned char*)Tptr - (unsigned char*)buff << std::endl;
#endif
			return new(Tptr) T();
		}

		template<typename T1, typename T2, typename... Ts>
		inline void __constructOnChunk_Internal() noexcept
		{
			__constructOnChunk_Internal<T1>();
			__constructOnChunk_Internal<T2, Ts...>();
		}

		// Returns the index in chunk.
		template<typename... Ts>
		inline int ConstructionOnChunk(const SEntity&& entity) noexcept
		{
			*(SEntity*)entityWrkPtr = entity;
			__constructOnChunk_Internal<Ts...>();
			if (!__offsetWrkptrs(1)) // Full
			{
				properties->ArcheType->freeChunk = nullptr;		
#if defined(DEBUG) || defined(_DEBUG)
				std::cout << "CHUNK FULL!" << std::endl;
#endif
			}
			return properties->Count - properties->FreeUnits - 1;
		}
	};


}

