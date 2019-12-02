#pragma once
#include "D3DCommon.h"
#include "SRenderTarget.hpp"


namespace SGraphics
{
	class __dx12Pass
	{
	public:
		__dx12Pass(ID3D12Device* device)
			:mDevice(device)
		{

		}
		__dx12Pass(ID3D12Device* device, ID3DBlob* vs, ID3DBlob* ps, 
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
			:mDevice(device), PS(ps), VS(vs), mInputLayout(inputLayout)
		{

		}
		__dx12Pass(ID3D12Device* device, const std::wstring& vsPath, const std::string& vsTarg,
			const std::wstring& psPath, const std::string& psTarg,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
		{
			VS = d3dUtil::CompileShader(vsPath, nullptr, vsTarg, "vs_5_1");
			PS = d3dUtil::CompileShader(psPath, nullptr, psTarg, "ps_5_1");
			mDevice = device;
			mInputLayout = inputLayout;
		}
		__dx12Pass(const __dx12Pass&& rhs) = delete;
		__dx12Pass(const __dx12Pass& rhs) = delete;
		__dx12Pass& operator=(const __dx12Pass& rhs) = delete;
		virtual ~__dx12Pass() {}


		// Fill descriptor heaps
		virtual void BuildDescriptorHeaps() = 0;
		// Build root signature
		virtual void BuildRootSignature() = 0;
		//Build PSO
		virtual void BuildPSO() = 0;


		// Inject resources:
		// ! Directly copy.
		virtual void InjectSrvResources(std::vector<ID3D12Resource*> srvResources)
		{
			mSrvResources = srvResources;
		}

		virtual bool Initialize(std::vector<ID3D12Resource*> srvResources)
		{
			//mSrvDescriptorHeaps.resize(1);
			mSrvResources = srvResources;
			mCbvSrvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			BuildDescriptorHeaps();
			BuildRootSignature();
			BuildPSO();
			return true;
		}

		virtual bool StartUp(ID3D12GraphicsCommandList* cmdList)
		{
			return cmdList != nullptr;
		}

	protected:
		__dx12Pass() {}

		Microsoft::WRL::ComPtr<ID3D12Device> mDevice = nullptr;
		// Root signature.
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

		std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> mSrvDescriptorHeaps;
		std::vector<ID3D12Resource*> mSrvResources;

		UINT mCbvSrvDescriptorSize = 0;

		Microsoft::WRL::ComPtr<ID3DBlob> PS = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> VS = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;
		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	};


	class SDx12Pass : public __dx12Pass
	{
	public:
		SDx12Pass()
			:__dx12Pass() {}
		SDx12Pass(ID3D12Device* device)
			:__dx12Pass(device) {}
		SDx12Pass(ID3D12Device* device, ID3DBlob* vs, ID3DBlob* ps,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
			:__dx12Pass(device, vs, ps, inputLayout) {}
		SDx12Pass(ID3D12Device* device, const std::wstring& vsPath, const std::string& vsTarg,
			const std::wstring& psPath, const std::string& psTarg,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout)
			:__dx12Pass(device, vsPath, vsTarg, psPath, psTarg, inputLayout) {}
		

		// Called once per pass before drawing.
		virtual void BindPerPassResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, size_t passSrvNum) = 0;
		// Called once per render item before drawing.
		virtual void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SRenderItem* ri) = 0;
		// Bind RT for drawing

	public:
		// Current implementation would cause repeat drawing.
		virtual void PushRenderItems(std::vector<SRenderItem*> renderItems)
		{
			mRenderItems = renderItems;
		}

		virtual void Draw(ID3D12GraphicsCommandList* cmdList, 
			D3D12_CPU_DESCRIPTOR_HANDLE* dsv, 
			SFrameResource* frameRes, size_t passSrvNumOnFrameRes,
			D3D12_CPU_DESCRIPTOR_HANDLE* rtvs, size_t rtv_num)
		{
			// positive-z
			//if(dsv)
			//	cmdList->ClearDepthStencilView(*dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0.f, 0.f, nullptr);
			cmdList->OMSetRenderTargets(rtv_num, rtvs, true, dsv);

			// Set descriptor heaps:
			ID3D12DescriptorHeap** descriptorHeaps = new ID3D12DescriptorHeap* [mSrvDescriptorHeaps.size()];
			for(size_t i = 0; i < mSrvDescriptorHeaps.size(); i++)
				 descriptorHeaps[i] = mSrvDescriptorHeaps[i].Get();

			cmdList->SetPipelineState(mPSO.Get());
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

		virtual void Draw(ID3D12GraphicsCommandList* cmdList,
			D3D12_CPU_DESCRIPTOR_HANDLE* dsv,
			SFrameResource* frameRes, 
			D3D12_CPU_DESCRIPTOR_HANDLE* rtvs, size_t rtv_num)
		{
			this->Draw(cmdList, dsv, frameRes, 0, rtvs, rtv_num);
		}
	protected:
		std::vector<SRenderItem*> mRenderItems;
	};
}
