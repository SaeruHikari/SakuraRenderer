#pragma once
#include "../SakuraSceneNode.hpp"
#include "../../Object/SStaticMesh.h"
#include "Framework/Managers/Scene/SSceneManager.h"
#include <functional>
#include <DirectXMathConvert.inl>

using namespace SScene;
using namespace DirectX;

namespace SEngine
{
	class SStaticMeshNode : public SakuraSceneNode
	{
	public:
		SStaticMeshNode(SakuraMath::SVector location = { 0.f, 0.f, 0.f })
		{
			std::shared_ptr<SStaticMesh> Mesh = std::make_shared<SStaticMesh>();
			SetLocation(location);
			// Upload
			i_renderItem = pSceneManager->RegistMesh(Mesh.get());
		}

		virtual void Tick(double deltaTime) override
		{
			// Test
			static double T = 0;
			T += deltaTime;
			SakuraSceneNode::Tick(deltaTime);
			if(auto renderItem = GetRenderItem())
			{
				renderItem->dxRenderItem.PrevWorld = renderItem->dxRenderItem.World;
				XMStoreFloat4x4(&(renderItem->dxRenderItem.World),
					XMMatrixScaling(NodeTransform.Scale.x, NodeTransform.Scale.y, NodeTransform.Scale.z) *
					XMMatrixTranslation(NodeTransform.Location.x, NodeTransform.Location.y, NodeTransform.Location.z));
				renderItem->dxRenderItem.NumFramesDirty = gNumFrameResources;
			}
		}
	public:
		SRenderItem* GetRenderItem()
		{
			return pSceneManager->GetRenderItem(i_renderItem);
		}
		SMaterial* GetMaterial()
		{
			return pSceneManager->GetMaterial(i_material);
		}
	protected:
		// properties are unique refed in scene manager class.
		SIndex i_renderItem;
		SIndex i_material;
	};
}