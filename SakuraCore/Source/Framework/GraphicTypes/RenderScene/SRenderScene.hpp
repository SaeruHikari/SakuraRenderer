#pragma once
#include "Interface/ISObject.h"
#include "Framework/GraphicTypes/D3D12/DX12Definations.h"
#include "Framework/Core/Object/SMaterial.h"

namespace SGraphics
{
	class SRenderItem : SImplements SakuraCore::ISSlientObject
	{
	public:
		SRenderItem()
		{
			dxRenderItem = SDxRenderItem();
			ISSlientObject::ISSlientObject();
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
		// Finalize function.
		virtual void Finalize()
		{

		}
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime)
		{

		}
		std::vector<std::unique_ptr<SRenderItem>> OpaqueRItems;
		std::unordered_map<std::string, std::unique_ptr<SMaterial>> OpaqueMaterials;
	};
}