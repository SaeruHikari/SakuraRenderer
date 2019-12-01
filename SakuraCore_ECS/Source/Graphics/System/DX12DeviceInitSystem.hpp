#pragma once
#include "../../pch.h"
#include "Graphics/Utils/DX12SystemUtils.hpp"
#include "Graphics/Utils/d3dx12.h"
#include "Graphics/Components/DX12DeviceComponent.h"

using namespace SECS;
using namespace Microsoft::WRL;

namespace SakuraGraphics 
{
	class DX12DeviceInitSystem : public SSystem
	{
	public:
		DECLARE_SYSTEM_INITIALIZE(DX12DeviceInitSystem, &DX12DeviceInitSystem::InitialSystem, this, DECLARE_SYSTEM_COMPONENTS(DX12DeviceComponent))
		void InitialSystem(SEntity entity, DX12DeviceComponent* _dx12Device)
		{
#if defined(DEBUG) || defined(_DEBUG)
			// Enable the D3D12 debug layer
			{
				ComPtr<ID3D12Debug> debugController;
				ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&(debugController))));
				debugController->EnableDebugLayer();
			}
#endif
			// Create DXGI(DirectX Graphics Interface) Factory.
			ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&_dx12Device->mdxgiFactory)));

			// Try to create hardware device.
			HRESULT hardwareResult = D3D12CreateDevice(
				nullptr,
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&_dx12Device->md3dDevice));

			// Fall back to WARP(Windows Advanced Rasterization Platform) device.
			if (FAILED(hardwareResult))
			{
				ComPtr<IDXGIAdapter> pWarpAdapter;
				ThrowIfFailed(_dx12Device->mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&(pWarpAdapter))));

				ThrowIfFailed(D3D12CreateDevice(
					pWarpAdapter.Get(),
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&_dx12Device->md3dDevice)));
			}

			// Create Fence
			ThrowIfFailed(_dx12Device->md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&_dx12Device->mFence)));

			// Cache descriptor sizes for after usage.
			_dx12Device->mRtvDescriptorSize = _dx12Device->md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			_dx12Device->mDsvDescriptorSize = _dx12Device->md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			_dx12Device->mCbvSrvUavDescriptorSize = _dx12Device->md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			// Check 4x MSAA quality for our back buffer point.
			// All Direct3D 11 capable devices support 4x MSAA for all render formats.
			// So we only need to check quality support.
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
			msQualityLevels.Format = _dx12Device->mBackBufferFormat;
			msQualityLevels.SampleCount = 8;
			msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
			msQualityLevels.NumQualityLevels = 0;
			ThrowIfFailed(_dx12Device->md3dDevice->CheckFeatureSupport(
				D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
				&msQualityLevels,
				sizeof(msQualityLevels)));
			_dx12Device->m4xMsaaQuality = msQualityLevels.NumQualityLevels;
			assert(_dx12Device->m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
			return;
		}
	};
}