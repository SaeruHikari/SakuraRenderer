#pragma once
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

namespace SGraphics
{
	class SsaoPass : public SDx12Pass
	{
	public:
		SsaoPass(ID3D12Device* device)
			:SDx12Pass(device)
		{

		}
		REFLECTION_ENABLE(SDx12Pass)
	public:
		virtual bool Initialize(std::vector<ID3D12Resource*> srvResources) override
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\Ssao.hlsl", nullptr, "PS", "ps_5_1");
			if (VS == nullptr)
				VS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\Ssao.hlsl", nullptr, "VS", "vs_5_1");
			mInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			// Startup SSAO .
			BuildKernalOffsets();

			return __dx12Pass::Initialize(srvResources);
		}

		virtual bool Initialize(std::vector<ComPtr<ID3D12DescriptorHeap>> descriptorHeaps)
		{
			if (PS == nullptr)
				PS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\Ssao.hlsl", nullptr, "PS", "ps_5_1");
			if (VS == nullptr)
				VS = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\Ssao.hlsl", nullptr, "VS", "vs_5_1");
			mInputLayout =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			// Startup SSAO .
			BuildKernalOffsets();

			return __dx12Pass::Initialize(descriptorHeaps);
		}

		virtual bool StartUp(ID3D12GraphicsCommandList* cmdList) override
		{
			BuildRandomVectorTexture(cmdList);
			return cmdList != nullptr;
		}

		// bind resource to srv heap ?
		void BuildDescriptorHeaps(std::vector<ID3D12Resource*> mSrvResources)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = 3;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; 
			ThrowIfFailed(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeaps[0])));

			// Fill out the heap with actual descriptors:
			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeaps[0]->GetCPUDescriptorHandleForHeapStart());

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			srvDesc.Format = mSrvResources[0]->GetDesc().Format;
			//srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
			mDevice->CreateShaderResourceView(mSrvResources[0], &srvDesc, hDescriptor);

			hDescriptor.Offset(1, mCbvSrvDescriptorSize);

			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			mDevice->CreateShaderResourceView(mSrvResources[1], &srvDesc, hDescriptor);

			hDescriptor.Offset(1, mCbvSrvDescriptorSize);

			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			mDevice->CreateShaderResourceView(mRandomVectorMap.Get(), &srvDesc, hDescriptor);
		}

		void BuildPSO()
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc;

			// PSO for opaque objects.
			ZeroMemory(&ssaoPsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			ssaoPsoDesc.InputLayout = { mInputLayout.data(), ((UINT)mInputLayout.size()) };
			ssaoPsoDesc.pRootSignature = mRootSignature.Get();
			ssaoPsoDesc.VS =
			{
				reinterpret_cast<BYTE*>(VS->GetBufferPointer()),
				VS->GetBufferSize()
			};
			ssaoPsoDesc.PS =
			{
				reinterpret_cast<BYTE*>(PS->GetBufferPointer()),
				PS->GetBufferSize()
			};
			ssaoPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			ssaoPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			ssaoPsoDesc.SampleMask = UINT_MAX;
			ssaoPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			ssaoPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			ssaoPsoDesc.DepthStencilState.DepthEnable = false;
			ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			ssaoPsoDesc.NumRenderTargets = 1;
			ssaoPsoDesc.RTVFormats[0] = AoMapFormat;
			ssaoPsoDesc.SampleDesc.Count = 1;
			ssaoPsoDesc.SampleDesc.Quality = 0;
			ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

			ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&mPSO)));
		}

		void BuildRootSignature()
		{
			CD3DX12_DESCRIPTOR_RANGE texTable0;
			texTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				2,	// N D
				0, 0);
			CD3DX12_DESCRIPTOR_RANGE texTable1;
			texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				1,
				2, 0);

			CD3DX12_ROOT_PARAMETER slotRootParameters[3];

			slotRootParameters[0].InitAsConstantBufferView(0);
			slotRootParameters[1].InitAsDescriptorTable(1, &texTable0, D3D12_SHADER_VISIBILITY_PIXEL);
			// static
			slotRootParameters[2].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);

			const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
				0,
				D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

			const CD3DX12_STATIC_SAMPLER_DESC linearClmap(
				1,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

			const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
				2,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,
				D3D12_TEXTURE_ADDRESS_MODE_BORDER,
				D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0.f, 0);

			const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
				3,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP);

			std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers = 
				{ pointClamp, linearClmap, depthMapSam, linearWrap };

			// Create root signature
			CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(3, slotRootParameters,
				(UINT)staticSamplers.size(), staticSamplers.data(),
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
			Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
			HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

			if (errorBlob != nullptr)	
				::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			ThrowIfFailed(hr);
			ThrowIfFailed(mDevice->CreateRootSignature(
				0,
				serializedRootSig->GetBufferPointer(),
				serializedRootSig->GetBufferSize(),
				IID_PPV_ARGS(mRootSignature.GetAddressOf())));
		}

		// Only one RT target in SSAO (Quad)
		void BindPerPassResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, size_t passSrvNum)
		{
			auto passCB = frameResource->SsaoCB->Resource();
			cmdList->SetGraphicsRootConstantBufferView(0, passCB->GetGPUVirtualAddress());
			cmdList->SetGraphicsRootDescriptorTable(1, mSrvDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart());
			CD3DX12_GPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart());
			hDescriptor.Offset(2, mCbvSrvDescriptorSize);
			cmdList->SetGraphicsRootDescriptorTable(2, hDescriptor);
		}

		// No object CB.
		void BindPerRenderItemResource(ID3D12GraphicsCommandList* cmdList, SFrameResource* frameResource, SDxRenderItem* ri)
		{
			//...
		}

	private:
		// Build 14 rand offsets with static direction. 
		void BuildKernalOffsets()
		{

			mOffsets[0] = DirectX::XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
			mOffsets[1] = DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

			mOffsets[2] = DirectX::XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
			mOffsets[3] = DirectX::XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

			mOffsets[4] = DirectX::XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
			mOffsets[5] = DirectX::XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

			mOffsets[6] = DirectX::XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
			mOffsets[7] = DirectX::XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

			// 6 centers of cube faces
			mOffsets[8] = DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
			mOffsets[9] = DirectX::XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

			mOffsets[10] = DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
			mOffsets[11] = DirectX::XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

			mOffsets[12] = DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
			mOffsets[13] = DirectX::XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

			for (int i = 0; i < 14; ++i)
			{
				// Create random lengths in [0.25, 1.0].
				float s = MathHelper::RandF(0.25f, 1.0f);

				DirectX::XMVECTOR v = s * DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&mOffsets[i]));

				XMStoreFloat4(&mOffsets[i], v);
			}
		}

		void BuildRandomVectorTexture(ID3D12GraphicsCommandList* cmdList)
		{
			D3D12_RESOURCE_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Alignment = 0;
			texDesc.Width = 256;
			texDesc.Height = 256;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			ThrowIfFailed(mDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&mRandomVectorMap)));

			// Upload Heap:
			const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(mRandomVectorMap.Get(), 0, num2DSubresources);

			ThrowIfFailed(mDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&mRandomVectorMapUploadBuffer)));

			XMCOLOR initData[256 * 256];
			//
			for (size_t i = 0; i < 256; i++)
			{
				for (size_t j = 0; j < 256; j++)
				{
					// Produce rand vector in [0, 1]. decompress in GPU.
					XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

					initData[i * 256 + j] = XMCOLOR(v.x, v.y, v.z, 0.f);
				}
			}

			D3D12_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.pData = initData;
			subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
			subResourceData.SlicePitch = subResourceData.RowPitch * 256;

			// Schedule to copy the data to the default resource:
			// Transition to copy state
			cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRandomVectorMap.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
			// copy from upload buffer to default buffer.
			UpdateSubresources(cmdList, mRandomVectorMap.Get(), mRandomVectorMapUploadBuffer.Get(),
				0, 0, num2DSubresources, &subResourceData);
			cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRandomVectorMap.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		}
	public:
		inline ID3D12Resource* GetRandVecMap() { return mRandomVectorMap.Get(); }
		inline auto GetRandVecSrvResource() { return mRandomVectorMapUploadBuffer.Get(); }
		inline void GetOffsetVectors(DirectX::XMFLOAT4 offsets[14]) { std::copy(&mOffsets[0], &mOffsets[14], &offsets[0]); }

		std::vector<float> CalcGaussWeights(float sigma)
		{
			float twoSigma2 = 2.0f * sigma * sigma;

			// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
			// For example, for sigma = 3, the width of the bell curve is 
			int blurRadius = (int)ceil(2.0f * sigma);

			assert(blurRadius <= MaxBlurRadius);

			std::vector<float> weights;
			weights.resize(2 * blurRadius + 1);

			float weightSum = 0.0f;

			for (int i = -blurRadius; i <= blurRadius; ++i)
			{
				float x = (float)i;

				weights[i + blurRadius] = expf(-x * x / twoSigma2);

				weightSum += weights[i + blurRadius];
			}

			// Divide by the sum so all the weights add up to 1.0.
			for (int i = 0; i < weights.size(); ++i)
			{
				weights[i] /= weightSum;
			}

			return weights;
		}
	private:
		static const int MaxBlurRadius = 5;
		inline static const DXGI_FORMAT AoMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		inline static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		DirectX::XMFLOAT4 mOffsets[14];
		Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorMap;
		Microsoft::WRL::ComPtr<ID3D12Resource> mRandomVectorMapUploadBuffer;
	};
}
