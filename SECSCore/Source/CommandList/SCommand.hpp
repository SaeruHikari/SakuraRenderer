#pragma once
#include "..\Managers\SEntityManager.hpp"

namespace SECS
{
	struct SCommand
	{
		virtual void Execute(SEntityManager* entMng) = 0;
	};

	struct SDestoryEntityCommand : public SCommand
	{
		SDestoryEntityCommand() = default;
		SDestoryEntityCommand(SEntity& _entity)
			:entity(_entity)
		{
		
		}
		SEntity entity;
		void Execute(SEntityManager* entMng)
		{
			entMng->DestroyEntityOnChunk(entity);
		}
	};
}


