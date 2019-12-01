/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:	  Sakura scene class, contains all scene nodes and managers.
Details:
*******************************************************************************************/
#pragma once
#include "Interface/ISObject.h"
#include "SakuraSceneNode.hpp"
#include "../Entities/SakuraEntity.h"

namespace SakuraCore
{
	class SakuraScene : SImplements ISTickObject
	{
	public:
		// Implements base ISTickObject interfaces.
		virtual bool Initialize() override;
		virtual void Finalize() override;
		virtual void Tick(double deltaTime) override;

		// Add Entity to the entities root.
		virtual bool AddEntity(std::shared_ptr<SakuraEntity> childNode);
		// Find scene node
		virtual std::shared_ptr<SakuraEntity> FindEntity(SceneNodeID id);

	private:
		// !
		std::shared_ptr<SakuraEntity> EntitiesRoot;
	};
}
