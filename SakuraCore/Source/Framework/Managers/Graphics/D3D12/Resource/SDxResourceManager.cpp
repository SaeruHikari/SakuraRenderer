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
	CreateCommandObjects();
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
	else if(descriptorSize > 0)
	{
		desc.NumDescriptors = 1000;
		if (descriptorSize == mDeviceInformation->cbvSrvUavDescriptorSize)
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		else if (descriptorSize == mDeviceInformation->dsvDescriptorSize)
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		else if (descriptorSize == mDeviceInformation->rtvDescriptorSize)
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		else return nullptr;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		mDescriptorHeaps[name] = std::make_unique<SDescriptorHeap>(md3dDevice.Get(), descriptorSize, desc);
		return mDescriptorHeaps[name].get();
	}
	return nullptr;
}

bool SGraphics::SDxResourceManager::LoadTextures(std::wstring Filename, std::string registName)
{
	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	if (Filename.find(L".dds") != std::string::npos)
	{
		auto texMap = std::make_unique<SD3DTexture>();
		texMap->Name = registName;
		texMap->Filename = Filename;
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
			mCommandList.Get(), texMap->Filename.c_str(),
			texMap->Resource, texMap->UploadHeap));
		mTextures[texMap->Name] = std::move(texMap);
		// Execute the initialization commands.
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsList0[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsList0), cmdsList0);
		return true;
	}
	else if (Filename.find(L".hdr") != std::string::npos)
	{
		auto texMap = std::unique_ptr<SD3DTexture>(d3dUtil::LoadHDRTexture(md3dDevice.Get(),
				mCommandList.Get(), registName, Filename));
		mTextures[registName] = std::move(texMap);
		// Execute the initialization commands.
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsList0[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsList0), cmdsList0);
		return true;
	}
	return false;
}

SGraphics::ISTexture* SGraphics::SDxResourceManager::GetTexture(std::string registName)
{
	if (mTextures.find(registName) != mTextures.end())
		return mTextures[registName].get();
	else return false;
}

int SGraphics::SDxResourceManager::RegistNamedRenderTarget(std::string registName,
	ISRenderTargetProperties rtProp, 
	std::string targetRtvHeap, std::string targetSrvHeap)
{
	if (mRenderTargets.find(registName) != mRenderTargets.end())
		return false;
	switch (rtProp.rtType)
	{
	case ERenderTargetTypes::E_RT2D:
	{
		auto rt2d = std::make_unique<SDx12RenderTarget2D>(rtProp.mWidth, rtProp.mHeight,
			rtProp, rtProp.bScaleWithViewport);
		rt2d->BuildDescriptors(md3dDevice.Get(),
			GetOrAllocDescriptorHeap(targetRtvHeap, mDeviceInformation->rtvDescriptorSize),
			GetOrAllocDescriptorHeap(targetSrvHeap, mDeviceInformation->cbvSrvUavDescriptorSize));
		mRenderTargets[registName] = std::move(rt2d);
	}
		return 1;
	case ERenderTargetTypes::E_RT3D:
	{
		auto rt3d = std::make_unique<SDx12RenderTargetCube>(
			rtProp.mWidth, rtProp.mHeight, rtProp.mRtvFormat);
		rt3d->BuildDescriptors(md3dDevice.Get(),
			GetOrAllocDescriptorHeap(targetRtvHeap, mDeviceInformation->rtvDescriptorSize),
			GetOrAllocDescriptorHeap(targetSrvHeap, mDeviceInformation->cbvSrvUavDescriptorSize));
		mRenderTargets[registName] = std::move(rt3d);
	}
		return 1;
	default:
		return -1;
	}
	return -1;
}

SGraphics::ISRenderTarget* SGraphics::SDxResourceManager::GetRenderTarget(std::string registName)
{
	if (mRenderTargets.find(registName) == mRenderTargets.end())
		return nullptr;
	else
		return mRenderTargets[registName].get();
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

