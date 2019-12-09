/*****************************************************************************************
				             Copyrights   SaeruHikari
CreateDate:							2019.10.21
Description:	          D3D12 version Graphics Manager.
Details:		     
*******************************************************************************************/
#pragma once
#include "../CommonInterface/SakuraGraphicsManagerBase.h"
#include "Framework/GraphicTypes/D3D12/D3DCommon.h"

//Link necessary d3d12 libraries
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace SakuraCore;

namespace SGraphics
{
	class SakuraD3D12GraphicsManager : public SakuraGraphicsManagerBase
	{
	public:
		SakuraD3D12GraphicsManager()
			:SakuraGraphicsManagerBase() 
		{
		};
		SakuraD3D12GraphicsManager(const SakuraD3D12GraphicsManager& rhs) = delete;
		SakuraD3D12GraphicsManager& operator=(const SakuraD3D12GraphicsManager& rhs) = delete;
		~SakuraD3D12GraphicsManager();
	public:
		bool Get4xMsaaState() const;
		void Set4xMsaaState(bool value);
		float AspectRatio() const;
	public:
		inline auto GetRtvCPU(int offset)
		{
			auto rtvCPU = CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
			rtvCPU.Offset(offset, RtvDescriptorSize());
			return rtvCPU;
		}
	public:
		// Graphics manager interfaces
		virtual void Clear() {};
		virtual void Draw() {};
		virtual void Present() {};
		virtual bool Initialize() 
		{ 
			return InitDirect3D12();
		};
		virtual void Finalize() {};
		virtual void Tick(double deltaTime) = 0;
		virtual void OnResize(UINT Width, UINT Height) override;

		virtual void OnMouseDown(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnMouseMove(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnMouseUp(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnKeyDown(double deltaTime) = 0;

		UINT RtvDescriptorSize() { return mDeviceInformation->rtvDescriptorSize; }
		inline auto DsvDescriptorSize() { return mDeviceInformation->dsvDescriptorSize; }
		inline auto CbvSrvUavDescriptorSize() { return mDeviceInformation->cbvSrvUavDescriptorSize; }
		inline auto GetDevice() { return md3dDevice.Get(); }
		inline auto GetDirectCmdList() { return mCommandList.Get(); }
		inline void FlushCommandQueue();
		inline auto GetQueue() { return mCommandQueue.Get(); }
		inline auto GetAlloc() { return mDirectCmdListAlloc.Get(); }

	protected:
		// D3D12 methods. 
		bool InitDirect3D12();
		// Create Descriptor Heaps for RenderTargetView & Depth/Stencil View
		virtual void CreateRtvAndDsvDescriptorHeaps();
		void CreateCommandObjects();
		void CreateSwapChain();
		virtual bool CreateResourceManager();
		ID3D12Resource* CurrentBackBuffer() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

		void CalculateFrameStats();

		void LogAdapters();
	protected:
		// Set true to use 4xMSAA. THe default is false.
		bool	m4xMsaaState = false;	// 4xMSAA enabled.
		UINT    m4xMsaaQuality = 0;		// quality level of 4x MSAA

		// D3D12
		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

		static const int SwapChainBufferCount = 2;
		int mCurrBackBuffer = 0;
		int GBufferStart = 2;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

		D3D12_VIEWPORT mScreenViewport;
		D3D12_RECT mScissorRect;

		std::shared_ptr<SFence> mFence;
		std::shared_ptr<SDx12DeviceInformation> mDeviceInformation;
		std::shared_ptr<SDx12GraphicsStates> mGraphicsConfs;

#if defined(DEBUG) || defined(_DEBUG)
		Microsoft::WRL::ComPtr<IDXGISwapChain> mDbgSwapChain;
#endif
	};

}
