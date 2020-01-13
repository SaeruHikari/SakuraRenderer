#pragma once
#include "D3DCommon.h"
#include "..\GraphicsInterface\ISRenderPass.h"
#include "..\..\Core\Reflection\SakuraReflection.h"

namespace SGraphics
{
	class ISRenderTarget;
	class SRenderItem;
}

namespace SGraphics
{
	class __dx12Pass : SImplements ISRenderPass
	{
	public:
		__dx12Pass(ID3D12Device* device);
		__dx12Pass(ID3D12Device* device, ID3DBlob* vs, ID3DBlob* ps,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout);
		__dx12Pass(ID3D12Device* device, const std::wstring& vsPath, const std::string& vsTarg,
			const std::wstring& psPath, const std::string& psTarg,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout);
		__dx12Pass(const __dx12Pass&& rhs) = delete;
		__dx12Pass(const __dx12Pass& rhs) = delete;
		__dx12Pass& operator=(const __dx12Pass& rhs) = delete;
		virtual ~__dx12Pass() {}
		virtual void BuildDescriptorHeaps(std::vector<ID3D12Resource*> srvResources) = 0;
		virtual void BuildRootSignature() = 0;
		virtual void BuildPSO() = 0;
		virtual bool Initialize();
		virtual bool Initialize(std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> srvHeaps);
		virtual bool Initialize(std::vector<ID3D12Resource*> srvResources);
		virtual bool StartUp(ID3D12GraphicsCommandList* cmdList);

	protected:
		__dx12Pass() {}
		Microsoft::WRL::ComPtr<ID3D12Device> mDevice = nullptr;
		// Root signature.
		Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
		std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> mSrvDescriptorHeaps;
		UINT mCbvSrvDescriptorSize = 0;
		Microsoft::WRL::ComPtr<ID3DBlob> PS = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> VS = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;
		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	};

	class SDx12Pass : public SGraphics::__dx12Pass
	{
	public:
		SDx12Pass();
		SDx12Pass(ID3D12Device* device);
		SDx12Pass(ID3D12Device* device, ID3DBlob* vs, ID3DBlob* ps,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout);
		SDx12Pass(ID3D12Device* device, const std::wstring& vsPath, const std::string& vsTarg,
			const std::wstring& psPath, const std::string& psTarg,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout);
		
		// Called once per pass before drawing.
		virtual void BindPerPassResource(ID3D12GraphicsCommandList* cmdList, 
			SFrameResource* frameResource, size_t passSrvNum) = 0;
		// Called once per render item before drawing.
		virtual void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList,
			SFrameResource* frameResource, SDxRenderItem* ri) = 0;

		REFLECTION_ENABLE()
	public:
		// Current implementation would cause repeat drawing.
		virtual void PushRenderItems(std::vector<SGraphics::SRenderItem*> renderItems);

		virtual void Draw(ID3D12GraphicsCommandList* cmdList,
			D3D12_CPU_DESCRIPTOR_HANDLE* dsv,
			SFrameResource* frameRes,
			D3D12_CPU_DESCRIPTOR_HANDLE* rtvs, size_t rtv_num, size_t passSrvNumOnFrameRes = 0);

		virtual void Execute(ID3D12GraphicsCommandList* cmdList,
			D3D12_CPU_DESCRIPTOR_HANDLE* dsv,
			SFrameResource* frameRes,
			ISRenderTarget** rts, size_t rtv_num, size_t passSrvNumOnFrameRes = 0);

		virtual SCommandList* GetCmdList() override;
		virtual void ClearCmd() override;

	protected:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
		std::vector<SRenderItem*> mRenderItems;
	};
}
