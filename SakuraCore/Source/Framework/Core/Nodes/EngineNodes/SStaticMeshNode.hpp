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
		SStaticMeshNode()
		{
			std::shared_ptr<SStaticMesh> Mesh = std::make_shared<SStaticMesh>();
			// Upload
			renderItem = pSceneManager->AddOpaque(Mesh.get());
		}
		virtual void Tick(double deltaTime) override
		{
			static double T = 0;
			T += deltaTime;
			SakuraSceneNode::Tick(deltaTime);
			if(renderItem)
			{
				renderItem->dxRenderItem.PrevWorld = renderItem->dxRenderItem.World;
				XMStoreFloat4x4(&(renderItem->dxRenderItem.World), XMMatrixScaling(2.5f, 2.5f, 2.5f) *
					XMMatrixTranslation(2.7f * T, 2.7f, 0.f));
				renderItem->dxRenderItem.NumFramesDirty = gNumFrameResources;
			}
		}
		SRenderItem* renderItem = nullptr;
	};
}