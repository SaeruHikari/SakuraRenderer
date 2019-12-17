#pragma once
#include "../../CommonInterface/Resource/SakuraGraphicsResourceManagerBase.h"
#include "Framework/GraphicTypes/D3D12/SDescriptorHeap.hpp"

class SFrameResource;
namespace SGraphics
{
	class ISDx12RenderTarget;

	// CB: In Frame Resources
	// Here: Create Rtv Srv Descriptors
	class SDxResourceManager : public SakuraGraphicsResourceManagerBase
	{
	public:
		SDxResourceManager() = default;
		SDxResourceManager(Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory,
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
			std::shared_ptr<SFence> fence,
			std::shared_ptr<SDx12DeviceInformation> deviceInformation,
			std::shared_ptr<SDx12GraphicsStates> graphicsConfs);
		SDxResourceManager(const SDxResourceManager& rhs) = delete;
		SDxResourceManager& operator=(const SDxResourceManager&) = delete;
		~SDxResourceManager();
	public:
		// Initialize function.
		virtual bool Initialize();
		// Finalize function.
		virtual void Finalize();
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime);
		virtual SDescriptorHeap* GetOrAllocDescriptorHeap(std::string name,
			UINT descriptorSize = 0, 
			D3D12_DESCRIPTOR_HEAP_DESC desc = {});
		// Get Textures
		virtual ISRenderTarget* CreateNamedRenderTarget(std::string resgistName, ISRenderTargetProperties rtProp,
			std::string targetSrvHeap, std::string targetRtvHeap, SRHIResource* resource = nullptr) override;
		virtual ISRenderTarget* CreateNamedRenderTarget(std::string registName,
			ISRenderTargetProperties rtProp, SRHIResource* resource, SResourceHandle srvHandle, SResourceHandle rtvHandle) override;
	protected:
		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
		std::shared_ptr<SFence> mFence;
		std::shared_ptr<SDx12DeviceInformation> mDeviceInformation;
		std::shared_ptr<SDx12GraphicsStates> mGraphicsConfs;
		
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
		
		std::map<std::string, std::unique_ptr<SDescriptorHeap>> mDescriptorHeaps;

		bool InitD3D12Device();
		void CreateCommandObjects();
	protected:
		virtual ISTexture* LoadTexture(std::wstring Filename, std::string textName) override;
	};

}