#pragma once
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"

namespace SGraphics
{
	class SMotionVectorPass : public SDx12Pass
	{
	public:
		SMotionVectorPass(ID3D12Device* device)
			:SDx12Pass(device)
		{

		}
		virtual bool Initialize() override
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\MotionVectorPass.hlsl", nullptr, "PS", "ps_5_1");
			if (VS == nullptr)
				VS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\StandardVS.hlsl", nullptr, "VS", "vs_5_1");
			mInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			return __dx12Pass::Initialize();
		}


		// World Position
		void BuildDescriptorHeaps(std::vector<ID3D12Resource*> mSrvResources)
		{
			
		}

		virtual void Execute(ID3D12GraphicsCommandList* cmdList, D3D12_CPU_DESCRIPTOR_HANDLE* dsv, SFrameResource* frameRes,
			size_t passSrvNumOnFrameRes, ISRenderTarget** rtvs, size_t rtv_num)
		{
			cmdList->ClearRenderTargetView((*(rtvs))->GetRenderTargetHandle()->hCpu, Colors::Black, 0, nullptr);
			SDx12Pass::Execute(cmdList, dsv, frameRes, rtvs, rtv_num, passSrvNumOnFrameRes);
		}

		void BuildPSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferPsoDesc;

			// PSO for opaque objects.
			ZeroMemory(&gbufferPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			gbufferPsoDesc.InputLayout = { mInputLayout.data(), ((UINT)mInputLayout.size()) };
			gbufferPsoDesc.pRootSignature = mRootSignature.Get();
			gbufferPsoDesc.VS =
			{
				reinterpret_cast<BYTE*>(VS->GetBufferPointer()),
				VS->GetBufferSize()
			};
			gbufferPsoDesc.PS =
			{
				reinterpret_cast<BYTE*>(PS->GetBufferPointer()),
				PS->GetBufferSize()
			};
			gbufferPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

			gbufferPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			gbufferPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
#ifndef REVERSE_Z
			gbufferPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			gbufferPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
#else
			gbufferPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			gbufferPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
#endif 
			gbufferPsoDesc.SampleMask = UINT_MAX;
			gbufferPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			gbufferPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			gbufferPsoDesc.pRootSignature = mRootSignature.Get();
			gbufferPsoDesc.NumRenderTargets = 1;
			gbufferPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16_UNORM;
			gbufferPsoDesc.SampleDesc.Count = 1;
			gbufferPsoDesc.SampleDesc.Quality = 0;
			ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&gbufferPsoDesc, IID_PPV_ARGS(&mPSO)));
		}

		void BuildRootSignature()
		{
			// Root parameter can be a table, root descriptor or root constants.
			CD3DX12_ROOT_PARAMETER slotRootParameter[2];

			// Create root CBV
			slotRootParameter[0].InitAsConstantBufferView(0);
			slotRootParameter[1].InitAsConstantBufferView(1);

			auto staticSamplers = HikaD3DUtils::GetStaticSamplers();

			// A root signature is an array of root parameters.
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter,
				(UINT)staticSamplers.size(), staticSamplers.data(),
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// Create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer.
			Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

			if (errorBlob != nullptr)
			{
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			}
			ThrowIfFailed(hr);

			ThrowIfFailed(mDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(mRootSignature.GetAddressOf())));
		}

		void BindPerPassResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, size_t passSrvNum)
		{
			auto passCB = frameResource->PassCB->Resource();
			// Injected tex array
			cmdList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

			objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SRenderMeshConstants));

			objectCB = frameResource->ObjectCB->Resource();
		}

		void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SDxRenderItem* ri)
		{
			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;

			cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		}

	private:
		UINT objCBByteSize;
		ID3D12Resource* objectCB;
	};
}
