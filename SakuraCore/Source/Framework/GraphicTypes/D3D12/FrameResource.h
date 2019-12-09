#pragma once

#include "Utils/d3dUtil.h"
#include "Common/HikaUtils/HikaCommonUtils/MathHelper.h"
#include "UploadBuffer.h"
#include "UploadVertices.h"

struct SRenderMeshConstants
{
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 PrevWorld = MathHelper::Identity4x4();
};

struct SPassConstants
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.f, 0.f, 0.f };
	int AddOnMsg = -1.f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.f, 0.f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.f, 0.f };
	float NearZ = 0.f;
	float FarZ = 0.f;
	float TotalTime = 0.f;
	float DeltaTime = 0.f;
	DirectX::XMFLOAT4 AmbientLight = { 0.f, 0.f, 0.f, 1.f };
	DirectX::XMFLOAT2 Jitter = { 0.f, 0.f };
	DirectX::XMFLOAT2 Pad = { 0.f, 0.f };
	DirectX::XMFLOAT4X4 PrevViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 UnjitteredViewProj = MathHelper::Identity4x4();
	// Indices [0, NUM_DIR_LIGHTS] are directional lights;
	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHTS] are point lights;
	// indices [NUM_DIR_LIGHTS + NUM_POINT_LIGHTS, NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS]
	// are spot lights for a maximum of MaxLights per object.
	Light Lights[MaxLights];
};

// Ssao constants.
struct SsaoConstants
{
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Proj;
	DirectX::XMFLOAT4X4 InvProj;
	DirectX::XMFLOAT4X4 ProjTex;
	DirectX::XMFLOAT4   OffsetVectors[14];

	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	UINT AddOnMsg = -1;
	UINT Pad1;
	// Coordinates given in view space.
	float    OcclusionRadius = 0.5f;
	float    OcclusionFadeStart = 0.2f;
	float    OcclusionFadeEnd = 2.f;
	float    SurfaceEpsilon = 0.05f;
};

// TAA constants
struct TaaConstants
{
	DirectX::XMFLOAT4X4 JitterredProj;

};

// Stores the resources needed for the CPU to build the command lists
// for a frame.
// Strong coupling resources with pass
// Need refactoring.
struct SFrameResource
{
public:
	SFrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
	{
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

		PassCB = std::make_unique<UploadBuffer<SPassConstants>>(device, passCount, true);
		MaterialCB = std::make_unique<UploadBuffer<PBRMaterialConstants>>(device, materialCount, true);
		ObjectCB = std::make_unique<UploadBuffer<SRenderMeshConstants>>(device, objectCount, true);
		SsaoCB = std::make_unique<UploadBuffer<SsaoConstants>>(device, passCount, true);
	}
	SFrameResource(const SFrameResource& rhs) = delete;
	SFrameResource& operator=(const SFrameResource& rhs) = delete;
	~SFrameResource() {};

	// We cannot reset the allocator until the GPU is done processing the commands.
	// So each frame needs their own allocator.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;

	// We cannot update a cbuffer until the GPU is done processing the commands
	// that reference it. So each frame needs their own cbuffers.
	// std::unique_ptr<UploadBuffer<FrameConstants>> FrameCB = nullptr;
	std::unique_ptr<UploadBuffer<SPassConstants>> PassCB = nullptr;
	std::unique_ptr<UploadBuffer<PBRMaterialConstants>> MaterialCB = nullptr;
	std::unique_ptr<UploadBuffer<SRenderMeshConstants>> ObjectCB = nullptr;
	std::unique_ptr<UploadBuffer<SsaoConstants>> SsaoCB = nullptr;

	// Fence value to mark commands up to this fence point. This lets us
	// check if these frame resources are still use by the GPU.
	UINT64 Fence = 0;
};

