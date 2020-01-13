#pragma once
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"

namespace SGraphics
{
	class SBrdfLutPass : public SDx12Pass
	{
	public:
		SBrdfLutPass(ID3D12Device* device)
			:SDx12Pass(device)
		{

		}
		REFLECTION_ENABLE(SDx12Pass)
	public:
		virtual bool Initialize() override
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\BrdfLut.hlsl", nullptr, "PS", "ps_5_1");
			if (VS == nullptr)
				VS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\Ssao.hlsl", nullptr, "VS", "vs_5_1");
			mInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } 
			};
			return __dx12Pass::Initialize();
		}

		void BuildDescriptorHeaps(std::vector<ID3D12Resource*> mSrvResources)
		{

		}

		void BuildPSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC brdfPsoDesc;
			//
			// PSO for opaque objects.
			//
			ZeroMemory(&brdfPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			brdfPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
			brdfPsoDesc.pRootSignature = mRootSignature.Get();
			brdfPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			brdfPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			brdfPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			brdfPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
#ifndef REVERSE_Z
			brdfPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#else
			brdfPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
#endif 
			brdfPsoDesc.SampleMask = UINT_MAX;
			brdfPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			brdfPsoDesc.NumRenderTargets = 1;
			brdfPsoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
			brdfPsoDesc.SampleDesc.Count = 1;
			brdfPsoDesc.SampleDesc.Quality = 0;
			brdfPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			brdfPsoDesc.VS =
			{
				reinterpret_cast<BYTE*>(VS->GetBufferPointer()),
				VS->GetBufferSize()
			};
			brdfPsoDesc.PS =
			{
				reinterpret_cast<BYTE*>(PS->GetBufferPointer()),
				PS->GetBufferSize()
			};
			ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&brdfPsoDesc, IID_PPV_ARGS(&mPSO)));
		}

		void BuildRootSignature()
		{
			// Root parameter can be a table, root descriptor or root constants.
			CD3DX12_ROOT_PARAMETER slotRootParameter[3];

			// Perfomance TIP: Order from most frequent to least frequent.
			//slotRootParameter[0].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);//Texture
			slotRootParameter[0].InitAsConstantBufferView(0);   // obj Constants
			slotRootParameter[1].InitAsConstantBufferView(1);
			slotRootParameter[2].InitAsConstantBufferView(2);

			auto staticSamplers = HikaD3DUtils::GetStaticSamplers();

			// A root signature is an array of root parameters.
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameter,
				(UINT)staticSamplers.size(), staticSamplers.data(),
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
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
			matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

			objectCB = frameResource->ObjectCB->Resource();
			matCB = frameResource->MaterialCB->Resource();
		}

		void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SDxRenderItem* ri)
		{
			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
			D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

			cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(2, matCBAddress);
		}
	private:
		UINT objCBByteSize;
		UINT matCBByteSize;
		ID3D12Resource* objectCB;
		ID3D12Resource* matCB;
	};
}
