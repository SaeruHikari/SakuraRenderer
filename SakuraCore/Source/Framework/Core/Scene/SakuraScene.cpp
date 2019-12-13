#include "SakuraScene.h"
#include "..\Nodes\EngineNodes\SStaticMeshNode.hpp"

bool SScene::SakuraScene::Initialize()
{
	/*
	int MatCBInd = 0;
	for (size_t i = 0; i < 11; i++)
	{
		for (size_t j = 0; j < 11; j++)
		{
			std::string Name = "test" + std::to_string(i) + std::to_string(j);
			auto testM = new SMaterial();
			testM->data.Name = Name;
			testM->data.MatCBIndex = MatCBInd++;
			testM->data.MatConstants.DiffuseSrvHeapIndex = -1;
			testM->data.MatConstants.RMOSrvHeapIndex = -1;
			testM->data.MatConstants.SpecularSrvHeapIndex = -1;
			testM->data.MatConstants.NormalSrvHeapIndex = -1;
			testM->data.MatConstants.Roughness = .1f * (float)i;
			testM->data.MatConstants.Metallic = .1f * (float)j;
			testM->data.MatConstants.Metallic = 1.f;
			testM->data.MatConstants.BaseColor = XMFLOAT3(
				j == 0 ? Colors::PaleVioletRed :
				j == 1 ? Colors::Gold :
				j == 2 ? Colors::Firebrick :
				j == 3 ? Colors::Red :
				j == 4 ? Colors::Green :
				j == 5 ? Colors::LightBlue :
				j == 6 ? Colors::BlueViolet :
				j == 7 ? Colors::Blue :
				j == 8 ? Colors::Purple :
				j == 9 ? Colors::LightGray :
				Colors::DarkGray
			);
			pSceneManager->RegistOpaqueMat(testM, Name);
			SakuraMath::SVector location = { i * 2.5f, j * 2.5f, 0.f };
			auto meshNode = std::make_shared<SEngine::SStaticMeshNode>(location, Name);
			AddSceneNode(meshNode);
		}
	}*/
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
