#include "SakuraScene.h"
#include "..\Nodes\EngineNodes\SStaticMeshNode.hpp"
#include "..\..\GraphicTypes\GraphicsCommon\UploadVertices.h"
#include <Modules\Importers\SceneImporter.h>

bool SScene::SakuraScene::Initialize()
{
	//CreateTestSphereMatrix();
	auto node = SSceneImporter::ImportScene("Resources/Models/SM_Sponza.fbx");
	AddSceneNode(std::move(node));
	return true;
}

void SScene::SakuraScene::CreateTestSphereMatrix()
{
	auto meshNode = new SEngine::SStaticMeshNode({ 0.f,0.f,0.f }, "OnlyOneMesh");
	auto GeoGen = std::make_shared<GeometryGenerator>();
	std::vector<StandardVertex> vertices;
	std::vector<std::uint16_t> indices;
	std::vector<std::pair<SubmeshDesc, SubmeshGeometry>> subMeshDatas;
	subMeshDatas.resize(11 * 11);
	StaticMeshData sp;
	for (size_t i = 0; i < 11; i++)
	{
		for (size_t j = 0; j < 11; j++)
		{
			std::string Name = "test" + std::to_string(i) + std::to_string(j);
			auto testM = std::make_unique<SMaterial>();
			testM->data.Name = Name;
			testM->data.MatConstants.DiffuseSrvHeapIndex = -1;
			testM->data.MatConstants.RMOSrvHeapIndex = -1;
			testM->data.MatConstants.SpecularSrvHeapIndex = -1;
			testM->data.MatConstants.NormalSrvHeapIndex = -1;
			testM->data.MatConstants.Roughness = .1f * (float)i;
			testM->data.MatConstants.Metallic = .1f * (float)j;
			//testM->data.MatConstants.Metallic = 1.f;
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
			testM->data.MatConstants.BaseColor = XMFLOAT3(Colors::DarkGray);
			pSceneManager->RegistOpaqueMat(Name, testM->data);
			SakuraMath::SVector location = { i * 2.5f, j * 2.5f, 0.f };
			sp = GeoGen->CreateSphere(1.f, 20, 20, location.x, location.y, location.z);

			subMeshDatas[i * 11 + j].first.Name = Name;
			subMeshDatas[i * 11 + j].first.Mat = Name;
			subMeshDatas[i * 11 + j].second.BaseVertexLocation = vertices.size();
			subMeshDatas[i * 11 + j].second.StartIndexLocation = indices.size();
			subMeshDatas[i * 11 + j].second.IndexCount = sp.GetIndices16().size();
			subMeshDatas[i * 11 + j].second.VertexCount = sp.Vertices.size();

			std::vector<StandardVertex> spvertices((UINT)sp.Vertices.size());
			for (int i = 0; i < sp.Vertices.size(); ++i)
			{
				spvertices[i].Pos = sp.Vertices[i].Position;
				spvertices[i].TexC = sp.Vertices[i].TexC;
				spvertices[i].Normal = sp.Vertices[i].Normal;
				spvertices[i].TexC = sp.Vertices[i].TexC;
				spvertices[i].Tangent = sp.Vertices[i].TangentU;
			}

			vertices.insert(vertices.end(), spvertices.begin(), spvertices.end());
			indices.insert(indices.end(), sp.GetIndices16().begin(), sp.GetIndices16().end());
		}
	}
	meshNode->SetMesh(vertices, indices, "OnlyOneMesh", subMeshDatas);
	AddSceneNode(meshNode);
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

bool SScene::SakuraScene::AddSceneNode(SakuraSceneNode* childNode)
{
	if (EntitiesRoot == nullptr)
		EntitiesRoot = std::unique_ptr<SakuraSceneNode>(childNode);
	else 
		EntitiesRoot->Attach(childNode);
	return true;
}

bool SScene::SakuraScene::AddSceneNode(std::unique_ptr<SakuraSceneNode> childNode)
{
	if (EntitiesRoot == nullptr)
		EntitiesRoot = std::move(childNode);
	else
		EntitiesRoot->Attach(childNode.get());
	return true;
}

SScene::SakuraSceneNode* SScene::SakuraScene::FindSceneNode(SGuid id)
{
	return nullptr;
}
