#pragma once
#include "..\Managers\SArcheTypeManager.hpp"
#include "..\Managers\SEntityManager.hpp"
#include "..\CommandList\SCommandMachine.hpp"
#include "SSystemGroup.hpp"

namespace SECS
{
	class SWorld
	{
		friend SSystemGroup;
	public:
		SWorld() {};
		SWorld(std::string _Name)
			:Name(_Name)
		{
			ArcheTypeManager = new SArcheTypeManager();
			EntityManager = new SEntityManager();
			CommandMachine = new SCommandMachine();
		}

		static SWorld* CreateSWorld(std::string _Name)
		{
			SWorld::Worlds[_Name] = new SWorld(_Name);
			return Worlds[_Name];
		}

		static SWorld* GetWorld(std::string _Name)
		{
			return Worlds[_Name];
		}

		template<typename ... Cs>
		inline static SEntity CreateEntity(SWorld* world)
		{
			return world->CreateEntity<Cs...>();
		}

		template<typename ... Cs>
		inline SEntity CreateEntity()
		{
			SEntity _entity = EntityManager->CreateEntity<Cs...>(ArcheTypeManager);
			return _entity;
		}

		template<typename ... Cs>
		inline void Each(std::function<void(SEntity, Cs* ...)> func) noexcept
		{
			EntityManager->Each(func, ArcheTypeManager);
			CommandMachine->Execute(EntityManager);
		}

		inline bool DestoryEntity(SEntity& entity) 
		{
			if (EntityManager->DestoryEntity(entity))
			{
				CommandMachine->push_Command<SDestoryEntityCommand>(entity);
				return true;
			}
			return false;
		}

		inline bool IsEntityAlive(const SEntity& entity)
		{
			return EntityManager->IsEntityAlive(entity);
		}

		inline SArcheType* FindArcheType(const SEntity& entity)
		{
			return EntityManager->FindArcheType(entity);
		}

		template<typename T>
		inline T* FindComponent(const SEntity& entity)
		{
			return EntityManager->FindComponent<T>(entity);
		}

		template<typename ... Cs>
		inline SEntity AddComponent(SEntity& entity)
		{
			auto _arc = FindArcheType(entity);
			entity = EntityManager->AddComponent<Cs...>(entity, ArcheTypeManager);
			auto _arc_n = FindArcheType(entity);
			if(_arc_n->Is(_arc)) return entity;
			else
			{
				for (int i = 0; i < m_rootGroups.size(); i++)
				{
					SSystemGroup::SystemGroups[m_rootGroups[i]]->AddNewArchetypeData(_arc_n);
				}
			}
			return entity;
		}

		template<typename ... Cs>
		inline SEntity DestoryComponent(SEntity & entity)
		{
			auto _arc = FindArcheType(entity);
			entity = EntityManager->DestoryComponent<Cs...>(entity, ArcheTypeManager);
			auto _arc_n = FindArcheType(entity);
			if (_arc_n->Is(_arc)) return entity;
			else
			{
				for (int i = 0; i < m_rootGroups.size(); i++)
				{
					SSystemGroup::SystemGroups[m_rootGroups[i]]->AddNewArchetypeData(_arc_n);
				}
			}
			return entity;
		}

		inline bool IncludeSystemGroup(std::string SystemGroupName)
		{
			 SSystem* rootGroup = SSystemGroup::GetRootSystems(SystemGroupName);
			 if (rootGroup == nullptr) return false;
			 m_rootGroups.push_back(SystemGroupName);
#if defined(DEBUG) || defined(_DEBUG)
			 std::cout << "World " << Name << " added system group: " << SystemGroupName << " into tick group!" << std::endl;
#endif
			 rootGroup->CollectSystemBoostInfos(EntityManager, ArcheTypeManager);
			 rootGroup->Initialize(EntityManager);
			 return true;
		}

		// Tick function, merge it into your message loop.
		inline void TickSystemGroups()
		{
			for (int i = 0; i < m_rootGroups.size(); i++)
			{
				SSystemGroup::SystemGroups[m_rootGroups[i]]->Update(EntityManager);
			}
			CommandMachine->Execute(EntityManager);
		}

	private:
		static std::unordered_map<std::string, SWorld*> __InitWorldMap()
		{
			std::unordered_map<std::string, SWorld*> _worlds;
			return _worlds;
		}
		static inline std::unordered_map<std::string, SWorld*> Worlds = SWorld::__InitWorldMap();

	private:
		std::string Name;
		SCommandMachine* CommandMachine = nullptr;
		SArcheTypeManager* ArcheTypeManager = nullptr;
		SEntityManager* EntityManager = nullptr;
		std::vector<std::string> m_rootGroups;
	};
}