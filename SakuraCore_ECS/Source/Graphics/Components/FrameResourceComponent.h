#pragma once
#include "Source/Types/SComponent.hpp"
#include "../Utils/MathHelper.h"

using namespace SECS;
#define MaxLights 16

namespace SakuraGraphics 
{
	struct PassConstants
	{
		DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
		DirectX::XMFLOAT3 EyePosW = { 0.f, 0.f, 0.f };
		float cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2 RenderTargetSize = { 0.f, 0.f };
		DirectX::XMFLOAT2 InvRenderTargetSize = { 0.f, 0.f };
		float NearZ = 0.f;
		float FarZ = 0.f;
		float TotalTime = 0.f;
		float DeltaTime = 0.f;
		DirectX::XMFLOAT4 AmbientLight = { 0.f, 0.f, 0.f, 1.f };

		// Indices [0, NUM_DIR_LIGHTS] are directional lights;
		// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHTS] are point lights;
		// indices [NUM_DIR_LIGHTS + NUM_POINT_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS]
		// are spot lights for a maximum of MaxLights per object.
		Light Lights[MaxLights];
	};
}


namespace SakuraGraphics {
	struct DX12FrameResourceComponent : public SComponent
	{
		// We cannot reset the allocator until the GPU is done processing the commands.
		// So each frame needs their own allocator.
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

		// We cannot update a cbuffer until the GPU is done processing the commands
		// that reference it. So each frame needs their own cbuffers.
		// std::unique_ptr<UploadBuffer<FrameConstants>> FrameCB = nullptr;
		std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
		std::unique_ptr<UploadBuffer<DisneyMaterialConstants>> MaterialCB = nullptr;
		std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;

		// Fence value to mark commands up to this fence point. This lets us
		// check if these frame resources are still use by the GPU.
		UINT64 Fence = 0;
	};
}