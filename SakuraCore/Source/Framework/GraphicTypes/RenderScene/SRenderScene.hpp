#pragma once
#include "Framework/Interface/ISObject.h"
#include "Framework/GraphicTypes/D3D12/DX12Definations.h"
#include "Framework/Core/Object/SMaterial.h"
#include "../D3D12/FrameResource.h"

namespace SGraphics
{
	enum ERenderLayer
	{
		E_Opaque = 0,
		E_ScreenQuad = 1,
		E_GBufferDebug = 2,
		E_SKY = 3,
		E_Cube = 4,
		E_Count = 5
	};

	class SRenderItem : SImplements SakuraCore::ISSilentObject
	{
	public:
		SRenderItem()
		{
			dxRenderItem = SDxRenderItem();
			ISSilentObject::ISSilentObject();
		}
		union
		{
			SDxRenderItem dxRenderItem;
		};
	};
	class SRenderScene : SImplements SakuraCore::ISTickObject
	{
	public:
		SRenderScene() = default;
		// Initialize function.
		virtual bool Initialize()
		{

			return true;
		}

		virtual SMaterial* RegistOpaqueMaterial(OpaqueMaterial& opaqueMat)
		{
			auto Mat = std::make_unique<SMaterial>();
			opaqueMat.MatCBIndex = OpaqueMaterials.size();
			Mat->data = opaqueMat;
			OpaqueMaterials[opaqueMat.Name] = std::move(Mat);
			return OpaqueMaterials[opaqueMat.Name].get();
		}

		virtual SMaterial* RegistOpaqueMaterial(const std::string& name, OpaqueMaterial& opaqueMat)
		{
			auto Mat = std::make_unique<SMaterial>();
			opaqueMat.MatCBIndex = OpaqueMaterials.size();
			Mat->data = opaqueMat;
			OpaqueMaterials[name] = std::move(Mat);
			return OpaqueMaterials[name].get();
		}

		virtual int RegistRenderItem(SDxRenderItem& renderItem,
			ERenderLayer renderLayer = ERenderLayer::E_Opaque)
		{
			auto SRitem = std::make_unique<SRenderItem>();
			renderItem.ObjCBIndex = mAllRItems.size();
			SRitem->dxRenderItem = renderItem;
			mRenderLayers[renderLayer].push_back(SRitem.get());
			mAllRItems.push_back(std::move(SRitem));
			return mAllRItems.size() - 1;
		}

		const std::vector<SRenderItem*>& GetRenderLayer(ERenderLayer renderLayer = ERenderLayer::E_Opaque)
		{
			return mRenderLayers[renderLayer];
		}

		// Finalize function.
		virtual void Finalize()
		{

		}
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime)
		{

		}
		SFrameResource* GetFrameResource(size_t index)
		{
			return mFrameResources[index].get();
		}
		SMaterial* GetMaterial(const std::string& name)
		{
			return OpaqueMaterials[name].get();
		}

		std::vector<std::unique_ptr<SFrameResource>> mFrameResources;

		std::vector<std::unique_ptr<SRenderItem>> mAllRItems;
		// Render items divided by PSO
		std::vector<SRenderItem*> mRenderLayers[ERenderLayer::E_Count];

		// Material values 
		std::unordered_map<std::string, std::unique_ptr<SMaterial>> OpaqueMaterials;
	};
}