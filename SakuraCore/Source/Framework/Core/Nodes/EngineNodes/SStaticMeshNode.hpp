/*****************************************************************************************
							 Copyrights   SaeruHikari
CreateDate:							2020.1.1
Description:	       A kind of node stands for static mesh.
Details:
*******************************************************************************************/
#pragma once
#include "../SakuraSceneNode.hpp"
#include "../../Object/SStaticMesh.h"
#include "Framework/Managers/Scene/SSceneManager.h"
#include <functional>
#include <DirectXMathConvert.inl>
#include "../../../Managers/Graphics/D3D12/SDxRendererGM.h"

using namespace SScene;
using namespace DirectX;

namespace SEngine
{
	class SStaticSubmesh : public ISSilentObject
	{
	public:
		SStaticSubmesh() = default;
		SStaticSubmesh(SakuraSceneNode* _ownerNode, const std::string& _name,
			const std::string& matName = "DefaultMat")
			:i_material(matName), name(_name), ownerNode(_ownerNode)
		{
			// Upload
			i_renderItem = pSceneManager->RegistRenderItem(ownerNode->GetNodeName(), _name, matName);
		}

	public:
		__forceinline SRenderItem* GetRenderItem()
		{
			return pSceneManager->GetRenderItem(i_renderItem);
		}
		__forceinline SMaterial* GetMaterial()
		{
			return pSceneManager->GetMaterial(i_material);
		}
		__forceinline const std::string& GetMaterialName()
		{
			return i_material;
		}
		__forceinline void SetMaterialName(const std::string& matName)
		{
			i_material = matName;
		}
		__forceinline const std::string& GetName()
		{
			return name;
		}
		__forceinline void SetName(const std::string& newName)
		{
			name = newName;
		}
		REFLECTION_ENABLE(ISSilentObject)
		virtual char* GetSClassName() override
		{
			return (char*)"SStaticSubmesh";
		};
	protected:
		SakuraSceneNode* ownerNode = nullptr;
		std::string name = "Submesh";
		SIndex i_renderItem;
		std::string i_material;
	};

	class SStaticMeshNode : public SakuraSceneNode
	{
	public:
		SStaticMeshNode(SakuraMath::SVector location = { 0.f, 0.f, 0.f }, const std::string& nodeName = "NULL")
		{
			NodeName = nodeName;
			SetLocation(location);
			UpdateTransform();
		}
		~SStaticMeshNode()
		{

		}
		void UpdateTransform();
		virtual void Tick(double deltaTime) override;
		void SetSubMeshes(const std::vector<SStaticSubmesh*>& in)
		{
			return;
		}
		const std::vector<SStaticSubmesh*>& GetSubMeshes()
		{
			std::vector<SStaticSubmesh*> res;
			res.resize(submeshes.size());
			for (size_t i = 0; i < submeshes.size(); i++)
			{
				res[i] = submeshes[i].get();
			}
			return res;
		}
	public:
		SStaticSubmesh* testsubmesh;

		template<typename Vertex, typename Index>
		void SetMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices,
			const std::string& geomName,
			const std::vector<std::pair<SubmeshDesc, SubmeshGeometry>>& submeshDatas)
		{
			auto gmng = (SDxRendererGM*)pGraphicsManager;
			gmng->GetResourceManager()->RegistGeometry(geomName, vertices, indices, submeshDatas);
			submeshes.resize(submeshDatas.size());
			for (size_t i = 0; i < submeshDatas.size(); i++)
			{
				submeshes[i] = std::make_unique<SStaticSubmesh>(this, submeshDatas[i].first.Name, 
					submeshDatas[i].first.Mat);
				testsubmesh = submeshes[i].get();
			}
			UpdateTransform();
		}

		void SetMesh(StaticMeshData* _data, const std::string& geomName,
			const std::vector<std::pair<SubmeshDesc, SubmeshGeometry>>& submeshDatas)
		{
			auto gmng = (SDxRendererGM*)pGraphicsManager;
			std::vector<std::uint16_t> indices;
			indices.insert(indices.end(), std::begin(_data->GetIndices16()), std::end(_data->GetIndices16()));

			std::vector<StandardVertex> vertices((UINT)_data->Vertices.size());
			for (int i = 0; i < _data->Vertices.size(); ++i)
			{
				vertices[i].Pos = _data->Vertices[i].Position;
				vertices[i].TexC = _data->Vertices[i].TexC;
				vertices[i].Normal = _data->Vertices[i].Normal;
				vertices[i].TexC = _data->Vertices[i].TexC;
				vertices[i].Tangent = _data->Vertices[i].TangentU;
			}

			gmng->GetResourceManager()->RegistGeometry(geomName, vertices, indices, submeshDatas);

			submeshes.resize(submeshDatas.size());
			for (size_t i = 0; i < submeshDatas.size(); i++)
			{
				submeshes[i] = std::make_unique<SStaticSubmesh>(this, submeshDatas[i].first.Name, submeshDatas[i].first.Mat);
				//!Debug
				testsubmesh = submeshes[i].get();
			}
			UpdateTransform();
		}
		REFLECTION_ENABLE(SakuraSceneNode)
		virtual char* GetSClassName() override
		{
			return (char*)"SStaticMeshNode";
		};
	protected:
		std::vector<std::unique_ptr<SStaticSubmesh>> submeshes;
	};
}