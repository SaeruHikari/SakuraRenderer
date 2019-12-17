#include "SDx12Pass.hpp"
#include "..\GraphicsInterface\ISRenderTarget.h"
#include "..\GraphicsInterface\ISRenderResource.h"

SGraphics::__dx12Pass::__dx12Pass(ID3D12Device* device, const std::wstring& vsPath, const std::string& vsTarg,
	const std::wstring& psPath, const std::string& psTarg,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
{
	VS = d3dUtil::CompileShader(vsPath, nullptr, vsTarg, "vs_5_1");
	PS = d3dUtil::CompileShader(psPath, nullptr, psTarg, "ps_5_1");
	mDevice = device;
	mInputLayout = inputLayout;
	mCbvSrvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

SGraphics::SDx12Pass::SDx12Pass()
	:__dx12Pass() {}

SGraphics::SDx12Pass::SDx12Pass(ID3D12Device* device, const std::wstring& vsPath, const std::string& vsTarg, const std::wstring& psPath, const std::string& psTarg, const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
	: __dx12Pass(device, vsPath, vsTarg, psPath, psTarg, inputLayout) {}

SGraphics::SDx12Pass::SDx12Pass(ID3D12Device* device, ID3DBlob* vs, ID3DBlob* ps, const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
	: __dx12Pass(device, vs, ps, inputLayout) {}

SGraphics::SDx12Pass::SDx12Pass(ID3D12Device* device)
	: __dx12Pass(device) {}


bool SGraphics::__dx12Pass::Initialize()
{
	BuildRootSignature();
	BuildPSO();
	return true;
}

bool SGraphics::__dx12Pass::StartUp(ID3D12GraphicsCommandList* cmdList)
{
	return cmdList != nullptr;
}

SGraphics::__dx12Pass::__dx12Pass(ID3D12Device* device, ID3DBlob* vs, ID3DBlob* ps, const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
	:mDevice(device), PS(ps), VS(vs), mInputLayout(inputLayout)
{
	mCbvSrvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

SGraphics::__dx12Pass::__dx12Pass(ID3D12Device* device)
	:mDevice(device)
{
	mCbvSrvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

}

bool SGraphics::__dx12Pass::Initialize(std::vector<ID3D12Resource*> srvResources)
{
	mSrvDescriptorHeaps.resize(1);
	BuildDescriptorHeaps(srvResources);
	return Initialize();
}

bool SGraphics::__dx12Pass::Initialize(std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> srvHeaps)
{
	mSrvDescriptorHeaps = srvHeaps;
	return Initialize();
}

void SGraphics::SDx12Pass::PushRenderItems(std::vector<SDxRenderItem*> renderItems)
{
	mRenderItems = renderItems;
}

void SGraphics::SDx12Pass::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE* dsv, SFrameResource* frameRes, D3D12_CPU_DESCRIPTOR_HANDLE* rtvs, size_t rtv_num, size_t passSrvNumOnFrameRes)
{
	cmdList->OMSetRenderTargets(rtv_num, rtvs, true, dsv);
	// Set descriptor heaps:
	ID3D12DescriptorHeap** descriptorHeaps = mSrvDescriptorHeaps.size() == 0
		? nullptr : new ID3D12DescriptorHeap * [mSrvDescriptorHeaps.size()];
	for (size_t i = 0; i < mSrvDescriptorHeaps.size(); i++)
		descriptorHeaps[i] = mSrvDescriptorHeaps[i].Get();
	cmdList->SetPipelineState(mPSO.Get());
	if (mSrvDescriptorHeaps.size() != 0)
		cmdList->SetDescriptorHeaps(mSrvDescriptorHeaps.size(), descriptorHeaps);
	cmdList->SetGraphicsRootSignature(mRootSignature.Get());
	// Bind resource for this pass...
	BindPerPassResource(cmdList, frameRes, passSrvNumOnFrameRes);
	// Draw
	for (size_t j = 0; j < mRenderItems.size(); ++j)
	{
		auto ri = mRenderItems[j];
		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);
		BindPerRenderItemResource(cmdList, frameRes, ri);
		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

void SGraphics::SDx12Pass::Execute(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE* dsv, SFrameResource* frameRes, 
	ISRenderTarget** rts, size_t rtv_num, size_t passSrvNumOnFrameRes)
{
	D3D12_CPU_DESCRIPTOR_HANDLE* rtvs = new D3D12_CPU_DESCRIPTOR_HANDLE[rtv_num];
	for (size_t i = 0; i < rtv_num; i++)
		rtvs[i] = rts[i]->GetRenderTargetHandle()->hCpu;
	Draw(cmdList, dsv, frameRes, rtvs, rtv_num, passSrvNumOnFrameRes);
	delete[] rtvs;
}

