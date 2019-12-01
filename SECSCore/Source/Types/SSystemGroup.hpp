#pragma once
#include <map>
#include "SSystem.h"

namespace SECS
{
	class SSystemGroup : public SSystem
	{
#define ADD_SYSTEM_SUBGROUP(__className, __groupName)\
	SubSys[__groupName] = new __className();\

		friend SWorld;
	public:
		SSystemGroup()
		{
			
		}

	private:
		static SSystem* GetRootSystems(std::string GroupName)
		{
			if (SystemGroups.find(GroupName) != SystemGroups.end())
				return SystemGroups[GroupName];
			else return nullptr;
		}
	public:
		inline void Initialize(SEntityManager* _entityManager)
		{
			auto iter = SubSys.begin();
			while (iter != SubSys.end())
			{
				iter->second->Initialize(_entityManager);
				iter++;
			}
		}
		inline void Execute(SEntityManager* _entityManager)
		{
			auto iter = SubSys.begin();
			while (iter != SubSys.end())
			{
				iter->second->Execute(_entityManager);
				iter++;
			}
		}
		inline void Update(SEntityManager* _entityManager)
		{
			auto iter = SubSys.begin();
			while (iter != SubSys.end())
			{
				iter->second->Update(_entityManager);
				iter++;
			}
		}
		inline void CollectSystemBoostInfos(SEntityManager* entm, SArcheTypeManager* arcm) 
		{
			auto iter = SubSys.begin();
			while (iter != SubSys.end())
			{
				iter->second->CollectSystemBoostInfos(entm, arcm);
				iter++;
			}
		}
		inline void AddNewArchetypeData(SArcheType* arcType)
		{
			auto iter = SubSys.begin();
			while (iter != SubSys.end())
			{
				iter->second->AddNewArchetypeData(arcType);
				iter++;
			}
		}
	protected:
		std::unordered_map<std::string, SSystem*> SubSys;
	};
	REGISTRY_SYSTEM_TO_ROOT_GROUP(SSystemGroup, SECSDefaultGroup);
}


