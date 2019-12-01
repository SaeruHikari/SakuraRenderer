/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:	          Arche Type Manager, Create and manage Arche types.
Details:    A specific combine of components on a single entity is called the ArcheType.
*******************************************************************************************/
#pragma once
#include "../Types/SArcheType.hpp"
#include "../Types/SChunk.hpp"
#include "../Types/Containers/SArcheTypeList.hpp"
#include <unordered_map>
#include "../Types/Containers/SChunkList.hpp"
#include "../TemplateUtils/TemplatePackUtils.hpp"

namespace SECS
{
	class SArcheTypeManager
	{
		friend class SEntityManager;
		friend class SWorld;
		friend class SSystem;
	protected:
		// ArcheType Map: <compcount, ArchetypeList>
		std::unordered_map<size_t, SArcheTypeList> m_TypeLookup;

	protected:
		// No filter version.
		template<typename ... Cs>
		inline SArcheTypeList CompsGetArcheTypes()
		{
			SArcheTypeList resList = SArcheTypeList();
			size_t* InHashes = TemplatePackUtils::GetHashes<Cs...>();
			size_t  InComps = sizeof...(Cs);
			// Iterate around map.
			for (auto _pair = m_TypeLookup.begin(); _pair != m_TypeLookup.end(); ++_pair)
			{
				if(_pair->first < InComps) continue;
				// Iterate
				size_t FocusHash = 0;
				for (int i = 0; i < _pair->second.size(); i++)
				{
					if (_pair->second[i]->typeHashes[0] > InHashes[0]) continue;
					for (int j = 0; j < _pair->second[i]->ComponentNum; j++)
					{
						if (_pair->second[i]->typeHashes[j] == InHashes[FocusHash])
						{
							FocusHash += 1;
						}
						// All match, return this archetype.
						if (FocusHash == InComps)
						{
							resList.push_back(_pair->second[i]);
							break;
						}
						// ArcheType Hash already bigger than in hash tail, break.
						if(_pair->second[i]->typeHashes[j] > InHashes[InComps-1])
							break;
						// else do nothing and step in next loop.
					}
				}
			}
			return resList;
		}

		template<typename ... Cs>
		inline SArcheType* GetArcheType(SEntity* e)
		{
			// Finding...
			SArcheType* _archetype = GetExistingArcheType<Cs...>(e);
			if (_archetype != nullptr)
			{
#if defined(DEBUG) || defined(_DEBUG)
				std::cout << "Found existed archeType, reuse it." << std::endl;
#endif
				return _archetype;
			}

			// Not find
			_archetype = CreateArcheType<Cs...>(e);
			return _archetype;
		}

		template<typename ... Cs>
		inline SArcheType* GetExistingArcheType(SEntity* e) noexcept
		{
			SArcheType* _archetype = nullptr;
			size_t compCount = sizeof...(Cs);
			if (m_TypeLookup.find(compCount) != m_TypeLookup.end())
			{
				// Match
				for (auto* e : m_TypeLookup[compCount])
				{
					if (TemplatePackUtils::match<Cs...>(e->typeHashes, e->ComponentNum))
					{
						_archetype = e;
						return _archetype;
					}
				}
			}
			return nullptr;
		}

		inline SArcheType* ArchetypeRegisted(const SArcheType* _arc)
		{
			if (m_TypeLookup.find(_arc->ComponentNum) != m_TypeLookup.end())
			{
				// Match
				for (auto* e : m_TypeLookup[_arc->ComponentNum])
				{
					if (e->Is(_arc))
					{
						return e;
					}
				}
			}
			return nullptr;
		}

		inline SArcheType* RegistArchetype(SArcheType* _arc)
		{
			m_TypeLookup[_arc->ComponentNum].push_back(_arc);
#if defined(_DEBUG) || defined(DEBUG)
			std::cout << "SArcheType Manager: Regist new archetype." << std::endl;
#endif
			return _arc;
		}

		template<typename ... Cs>
		inline SArcheType* CreateArcheType(SEntity* e)
		{
			SArcheType* Arche = new SArcheType();
			Arche->Init<Cs...>(e);	
			m_TypeLookup[Arche->ComponentNum].push_back(Arche);
			return Arche;
		}


		SChunk* GetFreeSChunk(SArcheType* arcType)
		{
			SChunk* _chunk = nullptr;
			if (arcType->chunks == nullptr)
			{
				_chunk = new SChunk();
				_chunk->InitChunkLayout(arcType);
				arcType->chunks = new SChunkList;
				_chunk->properties->ListIndex = 0;
				arcType->chunks->push_back(_chunk);
				arcType->freeChunk = _chunk;
			}	
			else if (arcType->freeChunk == nullptr)
			{
#if defined(DEBUG) || (_DEBUG)
				std::cout << "No free chunk, construct Chunk: ";
#endif
				_chunk = new SChunk();
				_chunk->InitChunkLayout(arcType);
				_chunk->properties->ListIndex = arcType->chunks->size();
				arcType->chunks->push_back(_chunk);
				arcType->freeChunk = _chunk;
			}
			else
			{
#if defined(DEBUG) || (_DEBUG)
				std::cout << "Found free chunk, construct on it: " << std::endl;
#endif
				_chunk = arcType->freeChunk;
			}
			return _chunk;
		};


	protected:
		SArcheTypeManager()
		{
		
		}
	};
}
