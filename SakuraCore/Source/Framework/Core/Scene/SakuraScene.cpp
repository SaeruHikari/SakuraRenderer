#include "SakuraScene.h"

bool SakuraCore::SakuraScene::Initialize()
{
	// Be careful! Entities root can't be attached to any other entities!
	// Also it can ever not call:
	//    SakuraSceneNode::Detach() & SakuraSceneNode::DetachAndFinalize
	// Root node always have a GUID of 0, and the ID generator avoids to gen 0
	// for all other Entities.
	//EntitiesRoot = std::make_shared<SakuraEntity>();
	EntitiesRoot->SetID(0);

	return true;
}

void SakuraCore::SakuraScene::Finalize()
{
	// Discard Entities root.
	EntitiesRoot.reset();

}

void SakuraCore::SakuraScene::Tick(double deltaTime)
{
	
}

bool SakuraCore::SakuraScene::AddEntity(std::shared_ptr<SakuraEntity> childNode)
{

	return false;
}

std::shared_ptr<SakuraCore::SakuraEntity> SakuraCore::SakuraScene::FindEntity(SceneNodeID id)
{

	return nullptr;
}
