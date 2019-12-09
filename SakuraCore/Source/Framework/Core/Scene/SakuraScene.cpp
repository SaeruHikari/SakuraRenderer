#include "SakuraScene.h"
#include "..\Nodes\EngineNodes\SStaticMeshNode.hpp"

bool SScene::SakuraScene::Initialize()
{
	for (size_t i = 0; i < 11; i++)
	{
		for (size_t j = 0; j < 11; j++)
		{
			SakuraMath::SVector location = { i * 2.5f, j * 2.5f, 0.f };
			auto meshNode = std::make_shared<SEngine::SStaticMeshNode>(location);
			AddSceneNode(meshNode);
		}
	}
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
	if(EntitiesRoot)
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
