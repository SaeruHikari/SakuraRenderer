#include "SakuraD3D12GraphicsManager.hpp"
#include "Resource\SDxResourceManager.h"


using namespace Microsoft::WRL;
using namespace std;

void SGraphics::SakuraD3D12GraphicsManager::OnResize(UINT Width, UINT Height)
{
	assert(md3dDevice);
	assert(mSwapChain);
	assert(mDirectCmdListAlloc);

	mGraphicsConfs->clientWidth = Width;
	mGraphicsConfs->clientHeight = Height;

	//Flush before changing any resources
	FlushCommandQueue();

	//Reset command list
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	//Release the previous resources we will be recreating
	for (int i = 0; i < SwapChainBufferCount; ++i)
		mSwapChainBuffer[i].Reset();
	mDepthStencilBuffer.Reset();

	//Resize the swap chain
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mGraphicsConfs->clientWidth, mGraphicsConfs->clientHeight,
		mGraphicsConfs->backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	//Create the render target view
	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle = GetRtvCPU(0);
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, RtvDescriptorSize());
	}

	//Create the depth/stencil buffer and view
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mGraphicsConfs->clientWidth;
	depthStencilDesc.Height = mGraphicsConfs->clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	//SSAO chapter requires an SRV to the depth buffer to read from
	//the depth buffer. Therefore, because we need to create two views to the same resource:
	//	1.SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//	2.DSV format: DXGI_FORMAT_D24_UNORM_S8_UINT
	//we need to create the depth buffer resource with a typeless format
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mGraphicsConfs->depthStencilFormat;
#if defined(REVERSE_Z)
	optClear.DepthStencil.Depth = 0.f;
#else
	optClear.DepthStencil.Depth = 1.f;
#endif
	optClear.DepthStencil.Stencil = 0.f;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));


	//Create descriptor to mip level 0 of entire resource using the format of the resource
	//Create depth/stencil view with depth/stencil buffer resource 
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mGraphicsConfs->depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	//Translation the resource from its initial state to be used as a depth buffer
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	//Execute the resize commands
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	//Wait until resize is complete
	FlushCommandQueue();

	//Update the viewport transform to cover the client area
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mGraphicsConfs->clientWidth);
	mScreenViewport.Height = static_cast<float>(mGraphicsConfs->clientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mGraphicsConfs->clientWidth, mGraphicsConfs->clientHeight };
}

bool SGraphics::SakuraD3D12GraphicsManager::InitDirect3D12()
{
	// Create S Wrappers
	mFence = std::make_shared<SFence>();
	mDeviceInformation = std::make_shared<SDx12DeviceInformation>();
	mGraphicsConfs = std::make_shared<SDx12GraphicsStates>();

#if defined(DEBUG) || defined(_DEBUG)
	// Enable the D3D12 debug layer
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
	// Create DXGI(DirectX Graphics Interface) Factory.
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&md3dDevice));

	// Fall back to WARP(Windows Advanced Rasterization Platform) device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&md3dDevice)));
	}

	// Create Fence
	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&(mFence->fence))));

	// Cache descriptor sizes for after usage.
	mDeviceInformation->rtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDeviceInformation->dsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mDeviceInformation->cbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create Resource Manager
	CreateResourceManager();

	// Check 4x MSAA quality for our back buffer point.
	// All Direct3D 11 capable devices support 4x MSAA for all render formats.
	// So we only need to check quality support.
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mGraphicsConfs->backBufferFormat;
	msQualityLevels.SampleCount = 8;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));
	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
#if defined(_DEBUG) | defined(DEBUG)
	LogAdapters();
#endif
	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();
	return true;
}

void SGraphics::SakuraD3D12GraphicsManager::CreateCommandObjects()
{
	// Create command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	// Create command allocator.
	ThrowIfFailed(md3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));

	// Create command list.
	ThrowIfFailed(md3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mDirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(mCommandList.GetAddressOf())));

	// Start off in a closed state. This is because the first time we refer
	// to the command list we will reset it, and it needs to be closed before
	// calling rest.
	mCommandList->Close();
}

void SGraphics::SakuraD3D12GraphicsManager::CreateSwapChain()
{
	// Release the previous swap chain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mGraphicsConfs->clientWidth;
	sd.BufferDesc.Height = mGraphicsConfs->clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mGraphicsConfs->backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));

#if defined(DEBUG) || defined(_DEBUG)
	if (mhDbgWnd != nullptr)
	{
		sd.OutputWindow = mhDbgWnd;
		// Note: Swap chain uses queue to perform flush.
		ThrowIfFailed(mdxgiFactory->CreateSwapChain(
			mCommandQueue.Get(),
			&sd,
			mDbgSwapChain.GetAddressOf()));
	}
#endif
}

bool SGraphics::SakuraD3D12GraphicsManager::CreateResourceManager()
{
	pGraphicsResourceManager = std::make_unique<SDxResourceManager>(mdxgiFactory, md3dDevice, mFence, mDeviceInformation, mGraphicsConfs);
	pGraphicsResourceManager->Initialize();
	return pGraphicsResourceManager != nullptr;
}

void SGraphics::SakuraD3D12GraphicsManager::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	mFence->currentFence++;

	// Add an instruction to the command queue to set a new fence point. Because we 
	// are on the GPU time line, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signare()
	ThrowIfFailed(mCommandQueue->Signal(mFence->fence.Get(), mFence->currentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (mFence->fence->GetCompletedValue() < mFence->currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current Fence.
		ThrowIfFailed(mFence->fence->SetEventOnCompletion(mFence->currentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* SGraphics::SakuraD3D12GraphicsManager::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SGraphics::SakuraD3D12GraphicsManager::CurrentBackBufferView() 
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(GetResourceManager()
		->GetOrAllocDescriptorHeap("DefaultRtv")->GetCPUtDescriptorHandle(mCurrBackBuffer));
}

D3D12_CPU_DESCRIPTOR_HANDLE SGraphics::SakuraD3D12GraphicsManager::DepthStencilView() const
{
	return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void SGraphics::SakuraD3D12GraphicsManager::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame. These stats
	// are appended to the window captain bar.
	static int frameCnt = 0;
	static float timeElasped = 0.f;

	frameCnt++;

}

void SGraphics::SakuraD3D12GraphicsManager::LogAdapters()
{

}

void SGraphics::SakuraD3D12GraphicsManager::CreateRtvAndDsvDescriptorHeaps()
{
	//Create render target view descriptor for swap chain buffers
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	((SDxResourceManager*)(pGraphicsResourceManager.get()))
		->GetOrAllocDescriptorHeap("DefaultRtvName", mDeviceInformation->cbvSrvUavDescriptorSize, rtvHeapDesc);

	//Create depth/stencil view descriptor 
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

SGraphics::SakuraD3D12GraphicsManager::~SakuraD3D12GraphicsManager()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool SGraphics::SakuraD3D12GraphicsManager::Get4xMsaaState() const
{
	return m4xMsaaState;
}

void SGraphics::SakuraD3D12GraphicsManager::Set4xMsaaState(bool value)
{
	m4xMsaaState = value;
}

float SGraphics::SakuraD3D12GraphicsManager::AspectRatio() const
{
	return static_cast<float>(mGraphicsConfs->clientWidth) / mGraphicsConfs->clientHeight;
}

