#pragma once
#include <windows.h>
#include <dxgiformat.h>
#include <d3d12.h>
#include "Common\Microsoft\d3dx12.h"
#include "Utils/d3dUtil.h"
#include "Common/HikaUtils/HikaCommonUtils/MathHelper.h"
#include "UploadBuffer.h"
#include "Common/HikaUtils/HikaCommonUtils/GeometryGenerator.h"
#include "FrameResource.h"
#include "Utils/MeshImporter.h"
#include "Dx12Samplers.h"
#include "Framework/Interface\ISObject.h"
#include "../GraphicsCommon/GraphicsConfigs.h"


namespace SGraphics
{
	inline static const size_t gPoolPageSize = 100;
	struct SFence : SImplements SakuraCore::ISSilentObject
	{
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		UINT64 currentFence;
	};

	struct SDx12DeviceInformation
	{
		UINT rtvDescriptorSize = 0;
		UINT dsvDescriptorSize = 0;
		UINT cbvSrvUavDescriptorSize = 0;
		// Derived class should set these in derived constructor to customize starting values.
		D3D_DRIVER_TYPE d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	};
	struct SDx12GraphicsStates
	{
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		int clientWidth = 800;
		int clientHeight = 600;
	};

}