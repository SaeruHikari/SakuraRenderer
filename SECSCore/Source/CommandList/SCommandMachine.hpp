#pragma once
#include "SCommand.hpp"
#include "SCommandList.hpp"

namespace SECS
{
	class SCommandMachine
	{
	public:
		inline void Execute(SEntityManager* entityManager)
		{
			for (int i = CommandList.size() - 1; i > 0; i--)
			{
				CommandList[i]->Execute(entityManager);
				delete CommandList[i];
				CommandList.pop_back();
			}
		}
		template<typename T>
		inline void push_Command(SEntity& entity)
		{
			SCommand* newCommand = new T(entity);
			CommandList.push_back(newCommand);
		}
	private:
		SCommandList CommandList;
	};
}
