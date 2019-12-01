#pragma once
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"

namespace SGraphics
{
	// Draw once to get cubemap at game start.
	class SHDR2CubeMapPass : public SDx12Pass
	{
	public:
		SHDR2CubeMapPass(ID3D12Device* device)
			:SDx12Pass(device)
		{

		}
		virtual bool Initialize(std::vector<ID3D12Resource*> srvResources) override
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\HDR2CubeMapPass.hlsl", nullptr, "PS", "ps_5_1");
			if (VS == nullptr)
				VS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\HDR2CubeMapPass.hlsl", nullptr, "VS", "vs_5_1");
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

		// bind resource to srv heap
		void BuildDescriptorHeaps()
		{
			mSrvDescriptorHeaps.resize(1);
			//
			// Create the SRV heap.
			//
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = 1;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeaps[0])));

			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());

			auto hdrTex = mSrvResources[0];
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = hdrTex->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = hdrTex->GetDesc().MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			mDevice->CreateShaderResourceView(hdrTex, &srvDesc, hDescriptor);
		}

		void BuildPSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc;
			//
			// PSO for opaque objects.
			//
			ZeroMemory(&skyPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			skyPsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
			skyPsoDesc.pRootSignature = mRootSignature.Get();
			skyPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			skyPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			skyPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			// Make sure the depth function is LESS_EQUAL and not just LESS.  
			// Otherwise, the normalized depth values at z = 1 (NDC) will 
			// fail the depth test if the depth buffer was cleared to 1.
			skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			
			skyPsoDesc.SampleMask = UINT_MAX;
			skyPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			skyPsoDesc.NumRenderTargets = 1;
			skyPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
			skyPsoDesc.SampleDesc.Count = 1;
			skyPsoDesc.SampleDesc.Quality = 0;
			skyPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			skyPsoDesc.VS =
			{
				reinterpret_cast<BYTE*>(VS->GetBufferPointer()),
				VS->GetBufferSize()
			};
			skyPsoDesc.PS =
			{
				reinterpret_cast<BYTE*>(PS->GetBufferPointer()),
				PS->GetBufferSize()
			};
			ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&skyPsoDesc, IID_PPV_ARGS(&mPSO)));
		}

		void BuildRootSignature()
		{
			CD3DX12_DESCRIPTOR_RANGE texTable0;
			texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

			// Root parameter can be a table, root descriptor or root constants.
			CD3DX12_ROOT_PARAMETER slotRootParameter[3];

			// Perfomance TIP: Order from most frequent to least frequent.
			//slotRootParameter[0].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);//Texture
			slotRootParameter[0].InitAsConstantBufferView(0);   // obj Constants
			slotRootParameter[1].InitAsConstantBufferView(1);   // Pass Constants
			slotRootParameter[2].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);//Texture

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
			UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SPassConstants));
			auto passCB = frameResource->PassCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + passSrvNum * passCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView(1, passCBAddress);
			// Injected tex array
			cmdList->SetGraphicsRootDescriptorTable(2, mSrvDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart());
			objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SRenderMeshConstants));
			objectCB = frameResource->ObjectCB->Resource();
		}

		void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SRenderItem* ri)
		{
			// ... NO
			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
		}
	private:
		UINT objCBByteSize;
		ID3D12Resource* objectCB;
	};
}