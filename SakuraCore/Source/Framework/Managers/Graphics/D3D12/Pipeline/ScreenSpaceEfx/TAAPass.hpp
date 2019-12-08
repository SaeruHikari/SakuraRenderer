#pragma once
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"
namespace SGraphics
{
	class STaaPass : public SDx12Pass
	{
	public:
		STaaPass(ID3D12Device* device)
			:SDx12Pass(device)
		{

		}
		virtual bool Initialize(std::vector<ID3D12Resource*> srvResources) override
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\TaaPass.hlsl", nullptr, "PS", "ps_5_1");
			if (VS == nullptr)
				VS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\ScreenQuadVS.hlsl", nullptr, "VS", "vs_5_1");
			mInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			return __dx12Pass::Initialize(srvResources);
		}

		// bind resource to srv heap
		void BuildDescriptorHeaps(std::vector<ID3D12Resource*> mSrvResources)
		{
			mSrvDescriptorHeaps.resize(1);
			//
			// Create the SRV heap.
			//
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = mSrvResources.size();
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeaps[0])));

			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			for (size_t i = 0; i < mSrvResources.size(); i++)
			{
				auto skyTex = mSrvResources[i];
				srvDesc.Format = i == (mSrvResources.size() - 1) ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : skyTex->GetDesc().Format;
				srvDesc.TextureCube.MipLevels = skyTex->GetDesc().MipLevels;
				mDevice->CreateShaderResourceView(skyTex, &srvDesc, hDescriptor);
				hDescriptor.Offset(1, mCbvSrvDescriptorSize);
			}
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
			skyPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			skyPsoDesc.SampleMask = UINT_MAX;
			skyPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			skyPsoDesc.NumRenderTargets = 2;
			skyPsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			skyPsoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
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
			CD3DX12_DESCRIPTOR_RANGE historyChain, texInputs, depth;
			historyChain.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
			texInputs.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0);
			depth.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0);
			// Root parameter can be a table, root descriptor or root constants.
			CD3DX12_ROOT_PARAMETER slotRootParameter[4];
			// Perfomance TIP: Order from most frequent to least frequent.
			slotRootParameter[0].InitAsConstantBufferView(0);   // Pass Constants
			slotRootParameter[1].InitAsDescriptorTable(1, &historyChain, D3D12_SHADER_VISIBILITY_PIXEL);//Texture
			slotRootParameter[2].InitAsDescriptorTable(1, &texInputs, D3D12_SHADER_VISIBILITY_PIXEL);//Texture
			slotRootParameter[3].InitAsDescriptorTable(1, &depth, D3D12_SHADER_VISIBILITY_PIXEL);//Depth

			auto staticSamplers = HikaD3DUtils::GetStaticSamplers();

			// A root signature is an array of root parameters.
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
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
	public:
		int ResourceIndex = 0;
		void BindPerPassResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, size_t passSrvNum)
		{
			auto passCB = frameResource->PassCB->Resource();
			cmdList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());
			// Injected tex array
			CD3DX12_GPU_DESCRIPTOR_HANDLE descriptor(mSrvDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart());
			descriptor.Offset(ResourceIndex, mCbvSrvDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(1, descriptor);
			descriptor.Offset(2 - ResourceIndex, mCbvSrvDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(2, descriptor);
			descriptor.Offset(2, mCbvSrvDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(3, descriptor);
			objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SRenderMeshConstants));
			objectCB = frameResource->ObjectCB->Resource();
		}

		void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SDxRenderItem* ri)
		{
			// ... NO
			
		}
	private:
		UINT objCBByteSize;
		ID3D12Resource* objectCB;
	};

	// 8x TAA
	static const double Halton_2[8] =
	{
		0.0,
		-1.0 / 2.0,
		1.0 / 2.0,
		-3.0 / 4.0,
		1.0 / 4.0,
		-1.0 / 4.0,
		3.0 / 4.0,
		-7.0 / 8.0
	};

	// 8x TAA
	static const double Halton_3[8] =
	{
		-1.0 / 3.0,
		1.0 / 3.0,
		-7.0 / 9.0,
		-1.0 / 9.0,
		5.0 / 9.0,
		-5.0 / 9.0,
		1.0 / 9.0,
		7.0 / 9.0
	};
}
