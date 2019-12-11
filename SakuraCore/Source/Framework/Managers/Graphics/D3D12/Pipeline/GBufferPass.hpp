#pragma once
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"

namespace SGraphics
{
	class SGBufferPass : public SDx12Pass
	{
	public:
		bool bWriteDepth = true;
		SGBufferPass(ID3D12Device* device, bool bwriteDpeth)
			:SDx12Pass(device), bWriteDepth(bwriteDpeth)
		{
		}
		virtual bool Initialize(std::vector<ID3D12Resource*> srvResources) override
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\GBuffer.hlsl", nullptr, "PS", "ps_5_1");
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
			return __dx12Pass::Initialize(srvResources);
		}
		virtual bool Initialize(std::vector<ComPtr<ID3D12DescriptorHeap>> descriporHeaps)
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\GBuffer.hlsl", nullptr, "PS", "ps_5_1");
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
			return __dx12Pass::Initialize(descriporHeaps);
		}
		int DebugNumSrvResource = 100;
		// bind resource to srv heap ?
		void BuildDescriptorHeaps(std::vector<ID3D12Resource*> mSrvResources)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			// !
			srvHeapDesc.NumDescriptors = DebugNumSrvResource;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeaps[0])));

			// Fill out the heap with actual descriptors:
			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			// LOD Clamp 0, ban nega val
			srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
			
			// Iterate
			for (int i = 0; i < mSrvResources.size(); i++)
			{
				auto resource = mSrvResources[i];
				srvDesc.Format = resource->GetDesc().Format;
				srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
				mDevice->CreateShaderResourceView(resource, &srvDesc, hDescriptor);
				hDescriptor.Offset(1, mCbvSrvDescriptorSize);
			}
		}

		void BuildPSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferPsoDesc;

			// PSO for opaque objects.
			ZeroMemory(&gbufferPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			gbufferPsoDesc.InputLayout = { mInputLayout.data(), mInputLayout.size() };
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
			if (!bWriteDepth)
			{
				gbufferPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			}
#ifndef REVERSE_Z
			gbufferPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#else
			gbufferPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
#endif 
			gbufferPsoDesc.SampleMask = UINT_MAX;
			gbufferPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			gbufferPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
			gbufferPsoDesc.pRootSignature = mRootSignature.Get();
			gbufferPsoDesc.NumRenderTargets = 4;
			for (int i = 0; i < 4; i++)
			{
				gbufferPsoDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
			}
			gbufferPsoDesc.SampleDesc.Count = 1;
			gbufferPsoDesc.SampleDesc.Quality = 0;
			ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&gbufferPsoDesc, IID_PPV_ARGS(&mPSO)));
		}

		void BuildRootSignature()
		{
			// Albedo RMO Spec Normal
			CD3DX12_DESCRIPTOR_RANGE texTable;
			texTable.Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				DebugNumSrvResource,  // number of descriptors
				0); // register t0

			// Root parameter can be a table, root descriptor or root constants.
			CD3DX12_ROOT_PARAMETER slotRootParameter[4];

			// Create root CBV
			slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
			slotRootParameter[1].InitAsConstantBufferView(0);
			slotRootParameter[2].InitAsConstantBufferView(1);
			slotRootParameter[3].InitAsConstantBufferView(2);

			auto staticSamplers = HikaD3DUtils::GetStaticSamplers();

			// A root signature is an array of root parameters.
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
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
			cmdList->SetGraphicsRootDescriptorTable(0, mSrvDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart());
			cmdList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

			objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SRenderMeshConstants));
			matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

			objectCB = frameResource->ObjectCB->Resource();
			matCB = frameResource->MaterialCB->Resource();
		}

		void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SDxRenderItem* ri)
		{
			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
			D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);
		}

	private:
		UINT objCBByteSize;
		UINT matCBByteSize;
		ID3D12Resource* objectCB;
		ID3D12Resource* matCB;

	};
}
