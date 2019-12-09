#include "SDxResourceManager.h"
#include "Framework/GraphicTypes/D3D12/D3DCommon.h"

using namespace Microsoft::WRL;

SGraphics::SDxResourceManager::SDxResourceManager(
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory,
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
	std::shared_ptr<SFence> fence,
	std::shared_ptr<SDx12DeviceInformation> deviceInformation,
	std::shared_ptr<SDx12GraphicsStates> graphicsConfs)
	:SakuraGraphicsResourceManagerBase(),
	mdxgiFactory(dxgiFactory), md3dDevice(d3dDevice),
	mFence(fence), mDeviceInformation(deviceInformation),
	mGraphicsConfs(graphicsConfs)
{

};

SGraphics::SDxResourceManager::~SDxResourceManager()
{

}

bool SGraphics::SDxResourceManager::Initialize()
{
	InitD3D12Device();
	return true;
}

void SGraphics::SDxResourceManager::Finalize()
{

}

void SGraphics::SDxResourceManager::Tick(double deltaTime)
{

}

SGraphics::SDescriptorHeap* SGraphics::SDxResourceManager::GetOrAllocDescriptorHeap(std::string name,
	UINT descriptorSize, D3D12_DESCRIPTOR_HEAP_DESC desc)
{
	if (auto descPtr = mDescriptorHeaps[name].get())
	{
		return descPtr;
	}
	else
	{
		mDescriptorHeaps[name] = std::make_unique<SDescriptorHeap>(md3dDevice.Get(), descriptorSize, desc);
		return mDescriptorHeaps[name].get();
	}
}

bool SGraphics::SDxResourceManager::InitD3D12Device()
{
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
}

void SGraphics::SDxResourceManager::CreateCommandObjects()
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

