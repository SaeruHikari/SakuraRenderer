#include "SakuraScene.h"
#include "..\Nodes\EngineNodes\SStaticMeshNode.hpp"

bool SScene::SakuraScene::Initialize()
{
	auto meshNode = std::make_shared<SEngine::SStaticMeshNode>();
	AddSceneNode(meshNode);
	return true;
}

void SScene::SakuraScene::Finalize()
{
	// Discard Entities root.
	EntitiesRoot.reset();
}

void SScene::SakuraScene::Tick(double deltaTime)
{
	SakuraSceneNode::Tick(deltaTime);
	EntitiesRoot->Tick(deltaTime);
}

bool SScene::SakuraScene::AddSceneNode(std::shared_ptr<SakuraSceneNode> childNode)
{
	if (EntitiesRoot == nullptr)
		EntitiesRoot = childNode;
	else 
		EntitiesRoot->Attach(std::move(childNode));
	return true;
}

std::shared_ptr<SScene::SakuraSceneNode> SScene::SakuraScene::FindSceneNode(SGuid id)
{

	return nullptr;
}
