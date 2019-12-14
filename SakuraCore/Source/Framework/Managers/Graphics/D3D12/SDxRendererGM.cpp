#include <minwindef.h>
#include "SDxRendererGM.h"
#include "../../../Core/Nodes/EngineNodes/SStaticMeshNode.hpp"
#include "../../../GraphicTypes/FrameGraph/SakuraFrameGraph.h"
#define Sakura_Full_Effects
//#define Sakura_Debug_PureColor

#define Sakura_MotionVector

#define Sakura_Defferred 
#define Sakura_SSAO 

#define Sakura_IBL
#define Sakura_IBL_HDR_INPUT

#define Sakura_TAA

#define Sakura_GBUFFER_DEBUG 

#ifndef Sakura_MotionVector
#undef Sakura_TAA
#endif
#if defined(Sakura_TAA)
#ifndef Sakura_MotionVector
#define Sakura_MotionVector
#endif
#endif

#include "Pipeline/SsaoPass.hpp"
#include "Pipeline/GBufferPass.hpp"
#include "Pipeline/IBL/SkySpherePass.hpp"
#include "Pipeline/IBL/HDR2CubeMapPass.hpp"
#include "Pipeline/ScreenSpaceEfx/TAAPass.hpp"
#include "Pipeline/IBL/CubeMapConvolutionPass.hpp"
#include "Pipeline/IBL/BRDFLutPass.hpp"
#include "Pipeline/MotionVectorPass.hpp"
#include "Pipeline/DeferredPass.hpp"
#include "Debug/GBufferDebugPass.hpp"

#define TINYEXR_IMPLEMENTATION
#include "Includes/tinyexr.h"

namespace SGraphics
{
	bool SDxRendererGM::Initialize()
	{
		if (!SakuraD3D12GraphicsManager::Initialize())
			return false;
		// Reset the command list to prep for initialization commands.
		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

		// Get the increment size of a descriptor in this heap type. This is hardware specific
		// so we have to query this information
		mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		mCamera.SetPosition(0.0f, 0.0f, 0.0f);
		BuildCubeFaceCamera(0.0f, 2.0f, 0.0f);

		LoadTextures();
		BuildGeometry();
		BuildMaterials();
		BuildRenderItems();
		BuildDescriptorHeaps();
		// ¡ü Do not have dependency on dx12 resources
		{
			{
#if defined(Sakura_MotionVector)
				ISRenderTargetProperties vprop;
				vprop.mRtvFormat = DXGI_FORMAT_R16G16_UNORM;
				vprop.bScaleWithViewport = true;
				vprop.rtType = ERenderTargetTypes::E_RT2D;
				vprop.mWidth = mGraphicsConfs->clientWidth;
				vprop.mHeight = mGraphicsConfs->clientHeight;
				if (GetResourceManager()->
					RegistNamedRenderTarget(RT2Ds::MotionVectorRTName, vprop, RTVs::ScreenEfxRtvName, SRVs::ScreenEfxSrvName) != -1)
				{
					mMotionVectorRT = (SDx12RenderTarget2D*)GetResourceManager()->GetRenderTarget(RT2Ds::MotionVectorRTName);
				}
#endif
			}
#if defined(Sakura_IBL)		
			ISRenderTargetProperties prop;
			prop.mRtvFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
			prop.rtType = ERenderTargetTypes::E_RT2D;
			prop.mWidth = 4096;
			prop.mHeight = 4096;
			if (GetResourceManager()->
				RegistNamedRenderTarget(RT2Ds::BrdfLutRTName, prop, RTVs::CaptureRtvName, SRVs::CaptureSrvName) != -1)
			{
				mBrdfLutRT2D = (SDx12RenderTarget2D*)GetResourceManager()->GetRenderTarget(RT2Ds::BrdfLutRTName);
			}
			UINT _sizeS = 2048;
			for (int j = 0; j < SkyCubeMips; j++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE convCubeRtvHandles[6];
				prop.mHeight = _sizeS;
				prop.mWidth = _sizeS;
				prop.mRtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
				prop.rtType = ERenderTargetTypes::E_RT3D;
				_sizeS /= 2;
				prop.bScaleWithViewport = false;
				if (GetResourceManager()->
					RegistNamedRenderTarget(RT3Ds::SkyCubeRTNames[j], prop, RTVs::DeferredRtvName, SRVs::DeferredSrvName) != -1)
				{
					mSkyCubeRT[j] = (SDx12RenderTargetCube*)GetResourceManager()->GetRenderTarget(RT3Ds::SkyCubeRTNames[j]);
				}
			}
			UINT size = 1024;
			CD3DX12_CPU_DESCRIPTOR_HANDLE cubeRtvHandles[6];
			for (int i = 0; i < SkyCubeConvFilterNum; i++)
			{
				if (i != 0)
				{
					prop.mHeight = size;
					prop.mWidth = size;
					size = size / 2;
				}
				else
				{
					prop.mHeight = 64;
					prop.mWidth = 64;
				}
				if (GetResourceManager()->
					RegistNamedRenderTarget(RT3Ds::ConvAndPrefilterNames[i], prop, RTVs::DeferredRtvName, SRVs::DeferredSrvName) != -1)
				{
					mConvAndPrefilterCubeRTs[i] = 
						(SDx12RenderTargetCube*)GetResourceManager()->GetRenderTarget(RT3Ds::ConvAndPrefilterNames[i]);
				}
			}
#endif
			// Velocity and History
#if defined(Sakura_TAA)
			mTaaRTs = new SDx12RenderTarget2D*[TAARtvsNum];
			for (int i = 0; i < TAARtvsNum; i++)
			{
				ISRenderTargetProperties vprop;
				vprop.mRtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				vprop.bScaleWithViewport = true;
				vprop.rtType = ERenderTargetTypes::E_RT2D;
				vprop.mWidth = mGraphicsConfs->clientWidth;
				vprop.mHeight = mGraphicsConfs->clientHeight;
				if (GetResourceManager()->
					RegistNamedRenderTarget(RT2Ds::TAARTNames[i], vprop, RTVs::ScreenEfxRtvName, SRVs::ScreenEfxSrvName) != -1)
				{
					mTaaRTs[i] = (SDx12RenderTarget2D*)GetResourceManager()->GetRenderTarget(RT2Ds::TAARTNames[i]);
				}
			}
#endif
			// Create resources depended by passes. 
#if defined(Sakura_Defferred)
			GBufferRTs = new SDx12RenderTarget2D* [GBufferRTNum];
			for (int i = 0; i < GBufferRTNum; i++)
			{
				ISRenderTargetProperties prop(0.f, 0.f, (i == 3) ? 1.f : 0.f, 0.f);
				prop.bScaleWithViewport = true;
				prop.rtType = ERenderTargetTypes::E_RT2D;
				prop.mWidth = mGraphicsConfs->clientWidth;
				prop.mHeight = mGraphicsConfs->clientHeight;
				prop.mRtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
				if (GetResourceManager()->
					RegistNamedRenderTarget(RT2Ds::GBufferRTNames[i], prop, RTVs::DeferredRtvName, SRVs::DeferredSrvName) != -1)
				{
					GBufferRTs[i] = (SDx12RenderTarget2D*)GetResourceManager()->GetRenderTarget(RT2Ds::GBufferRTNames[i]);
				}
			}
#endif
		}
		//¡ý Do have dependency on dx12 resources
		BindPassResources();

#if defined(Sakura_Defferred)
#if defined(Sakura_MotionVector)
		auto mGbufferPass = GetFrameGraph()->
			RegistNamedPassNode<SGBufferPass>(ConsistingPasses::GBufferPassName, md3dDevice.Get(), false);
#else
		auto mGbufferPass = GetResourceManager()->
			RegistNamedPassNode<SGBufferPass>(ConsistingPasses::GBufferPassName, md3dDevice.Get(), true);
#endif
		mGbufferPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Opaque]);
#endif
		BuildFrameResources();
		mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
#if defined(Sakura_Defferred)
		auto mDeferredPass = GetFrameGraph()->
			RegistNamedPassNode<SDeferredPass>(ConsistingPasses::DeferredPassName, md3dDevice.Get());
		mDeferredPass->PushRenderItems(mRenderLayers[SRenderLayers::E_ScreenQuad]);
#if defined(Sakura_GBUFFER_DEBUG)
		auto mGBufferDebugPass = GetFrameGraph()->
			RegistNamedPassNode<SGBufferDebugPass>(ConsistingPasses::GBufferDebugPassName, md3dDevice.Get());
		mGBufferDebugPass->PushRenderItems(mRenderLayers[SRenderLayers::E_GBufferDebug]);
#endif
#endif

#if defined(Sakura_MotionVector)
		auto mMotionVectorPass = GetFrameGraph()->
			RegistNamedPassNode<SMotionVectorPass>(ConsistingPasses::MotionVectorPassName, md3dDevice.Get());
		mMotionVectorPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Opaque]);
		mMotionVectorPass->Initialize();
#endif
#if defined(Sakura_TAA)
		auto mTaaPass = GetFrameGraph()->
			RegistNamedPassNode<STaaPass>(ConsistingPasses::TaaPassName, md3dDevice.Get());
		mTaaPass->PushRenderItems(mRenderLayers[SRenderLayers::E_ScreenQuad]);
#endif
#if defined(Sakura_SSAO)
		auto mSsaoPass = GetFrameGraph()->
			RegistNamedPassNode<SsaoPass>(ConsistingPasses::SsaoPassName, md3dDevice.Get());
		mSsaoPass->PushRenderItems(mRenderLayers[SRenderLayers::E_ScreenQuad]);
		mSsaoPass->StartUp(mCommandList.Get());
#endif
#if defined(Sakura_IBL)
		std::shared_ptr<SBrdfLutPass> brdfPass = nullptr;
		// Draw brdf Lut
		brdfPass = std::make_shared<SBrdfLutPass>(md3dDevice.Get());
		brdfPass->PushRenderItems(mRenderLayers[SRenderLayers::E_ScreenQuad]);
		brdfPass->Initialize();
		//Update the viewport transform to cover the client area

		auto mDrawSkyPass = GetFrameGraph()->
			RegistNamedPassNode<SkySpherePass>(ConsistingPasses::SkySpherePassName, md3dDevice.Get());
		mDrawSkyPass->PushRenderItems(mRenderLayers[SRenderLayers::E_SKY]);
#endif
#if defined(Sakura_IBL_HDR_INPUT)
		std::shared_ptr<SHDR2CubeMapPass> mHDRUnpackPass = nullptr;
		std::shared_ptr<SCubeMapConvPass> mCubeMapConvPass = nullptr;

		mHDRUnpackPass = std::make_shared<SHDR2CubeMapPass>(md3dDevice.Get());
		mCubeMapConvPass = std::make_shared<SCubeMapConvPass>(md3dDevice.Get());
		mHDRUnpackPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Cube]);
		mCubeMapConvPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Cube]);
		std::vector<ID3D12Resource*> mHDRResource;
		mHDRResource.push_back(((SD3DTexture*)GetResourceManager()->GetTexture(Textures::texNames[5]))->Resource.Get());
		mHDRUnpackPass->Initialize(mHDRResource);
#endif
		// Execute the initialization commands.
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsList[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);
		// Wait until initialization is complete
		FlushCommandQueue();
		OnResize(mGraphicsConfs->clientWidth, mGraphicsConfs->clientHeight);

		// Pre-Compute RTs
		auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		ThrowIfFailed(cmdListAlloc->Reset());
		// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
		// Reusing the command list reuses memory.
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), nullptr));
#if defined(Sakura_IBL)//
		mDrawSkyPass->Initialize(mSkyCubeResource);
		Tick(1/60.f);
#if defined(Sakura_IBL_HDR_INPUT)
		static int Init = 0;
		if (Init < 6)
		{
			mCommandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(mBrdfLutRT2D->mResource.Get(),
					D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
			mBrdfLutRT2D->ClearRenderTarget(mCommandList.Get());
			for (size_t i = 0; i < SkyCubeConvFilterNum; i++)
			{
				mCommandList->ResourceBarrier(1,
					&CD3DX12_RESOURCE_BARRIER::Transition(mConvAndPrefilterCubeRTs[i]->Resource(),
						D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
				mConvAndPrefilterCubeRTs[i]->ClearRenderTarget(mCommandList.Get());
			}
			for (size_t i = 0; i < SkyCubeMips; i++)
			{
				// Prepare to unpack HDRI map to cubemap.
				mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSkyCubeRT[i]->Resource(),
					D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
				mSkyCubeRT[i]->ClearRenderTarget(mCommandList.Get());
			}

			D3D12_VIEWPORT screenViewport0;
			D3D12_RECT scissorRect0;
			screenViewport0.TopLeftX = 0;
			screenViewport0.TopLeftY = 0;
			screenViewport0.Width = static_cast<float>(4096);
			screenViewport0.Height = static_cast<float>(4096);
			screenViewport0.MinDepth = 0.0f;
			screenViewport0.MaxDepth = 1.0f;
			scissorRect0 = { 0, 0, 4096, 4096 };
			mCommandList->RSSetViewports(1, &screenViewport0);
			mCommandList->RSSetScissorRects(1, &scissorRect0);
			brdfPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource,
				&GetResourceManager()->GetOrAllocDescriptorHeap(RTVs::CaptureRtvName)->GetCPUtDescriptorHandle(0), 1);
			mCommandList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(mBrdfLutRT2D->mResource.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));

			//Update the viewport transform to cover the client area
			D3D12_VIEWPORT screenViewport;
			D3D12_RECT scissorRect;
			screenViewport.TopLeftX = 0;
			screenViewport.TopLeftY = 0;
			screenViewport.Width = static_cast<float>(2048);
			screenViewport.Height = static_cast<float>(2048);
			screenViewport.MinDepth = 0.0f;
			screenViewport.MaxDepth = 1.0f;
			scissorRect = { 0, 0, 2048, 2048 };
			mCommandList->RSSetViewports(1, &screenViewport);
			mCommandList->RSSetScissorRects(1, &scissorRect);
			for (size_t i = 0; i < SkyCubeMips; i++)
			{
				SPassConstants cubeFacePassCB = mMainPassCB;
				cubeFacePassCB.RenderTargetSize =
					XMFLOAT2(mSkyCubeRT[i]->mProperties.mWidth,
						mSkyCubeRT[i]->mProperties.mHeight);
				cubeFacePassCB.InvRenderTargetSize =
					XMFLOAT2(1.0f / mSkyCubeRT[i]->mProperties.mWidth,
						1.0f / mSkyCubeRT[i]->mProperties.mHeight);
				screenViewport.Width = static_cast<float>(mSkyCubeRT[i]->mProperties.mWidth);
				screenViewport.Height = static_cast<float>(mSkyCubeRT[i]->mProperties.mHeight);
				screenViewport.MinDepth = 0.0f;
				screenViewport.MaxDepth = 1.0f;
				scissorRect = { 0, 0, (LONG)screenViewport.Width, (LONG)screenViewport.Height };
				mCommandList->RSSetViewports(1, &screenViewport);
				mCommandList->RSSetScissorRects(1, &scissorRect);
				// Cube map pass cbuffers are stored in elements 1-6.	
				D3D12_CPU_DESCRIPTOR_HANDLE skyRtvs;
				while (Init < 6)
				{
					XMMATRIX view = mCubeMapCamera[Init].GetView();
					XMMATRIX proj = mCubeMapCamera[Init].GetProj();

					XMMATRIX viewProj = XMMatrixMultiply(view, proj);
					XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
					XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
					XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

					XMStoreFloat4x4(&cubeFacePassCB.View, XMMatrixTranspose(view));
					XMStoreFloat4x4(&cubeFacePassCB.InvView, XMMatrixTranspose(invView));
					XMStoreFloat4x4(&cubeFacePassCB.Proj, XMMatrixTranspose(proj));
					XMStoreFloat4x4(&cubeFacePassCB.InvProj, XMMatrixTranspose(invProj));
					XMStoreFloat4x4(&cubeFacePassCB.ViewProj, XMMatrixTranspose(viewProj));
					XMStoreFloat4x4(&cubeFacePassCB.InvViewProj, XMMatrixTranspose(invViewProj));
					cubeFacePassCB.EyePosW = mCubeMapCamera[Init].GetPosition3f();
					auto currPassCB = mCurrFrameResource->PassCB.get();
					currPassCB->CopyData(1 + Init, cubeFacePassCB);
					skyRtvs = mSkyCubeRT[i]->Rtv(Init);
					mHDRUnpackPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource, 1 + Init, &skyRtvs, 1);
					Init++;
				}
				Init = 0;
			}
			mSkyCubeResource.resize(SkyCubeMips);
			for (size_t i = 0; i < SkyCubeMips; i++)
			{
				mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mSkyCubeRT[i]->Resource(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
				mSkyCubeResource[i] = mSkyCubeRT[i]->Resource();
			}
			// Initialize Convolution Pass
			mCubeMapConvPass->Initialize(mSkyCubeResource);

			for (size_t i = 0; i < SkyCubeConvFilterNum; i++)
			{
				Init = 0;
				screenViewport.TopLeftX = 0;
				screenViewport.TopLeftY = 0;
				screenViewport.Width = static_cast<float>(mConvAndPrefilterCubeRTs[i]->mProperties.mWidth);
				screenViewport.Height = static_cast<float>(mConvAndPrefilterCubeRTs[i]->mProperties.mHeight);
				screenViewport.MinDepth = 0.0f;
				screenViewport.MaxDepth = 1.0f;
				scissorRect = { 0, 0, (LONG)mConvAndPrefilterCubeRTs[i]->mProperties.mWidth,
					(LONG)mConvAndPrefilterCubeRTs[i]->mProperties.mHeight };
				mCommandList->RSSetViewports(1, &screenViewport);
				mCommandList->RSSetScissorRects(1, &scissorRect);
				// Prepare to draw convoluted cube map.
				// Change to RENDER_TARGET.

				SPassConstants convPassCB = mMainPassCB;
				convPassCB.AddOnMsg = i;
				while (Init < 6)
				{
					XMMATRIX view = mCubeMapCamera[Init].GetView();
					XMMATRIX proj = mCubeMapCamera[Init].GetProj();

					XMMATRIX viewProj = XMMatrixMultiply(view, proj);
					XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
					XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
					XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

					XMStoreFloat4x4(&convPassCB.View, XMMatrixTranspose(view));
					XMStoreFloat4x4(&convPassCB.InvView, XMMatrixTranspose(invView));
					XMStoreFloat4x4(&convPassCB.Proj, XMMatrixTranspose(proj));
					XMStoreFloat4x4(&convPassCB.InvProj, XMMatrixTranspose(invProj));
					XMStoreFloat4x4(&convPassCB.ViewProj, XMMatrixTranspose(viewProj));
					XMStoreFloat4x4(&convPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
					convPassCB.EyePosW = mCubeMapCamera[Init].GetPosition3f();
					convPassCB.RenderTargetSize = XMFLOAT2((float)screenViewport.Width,
						(float)screenViewport.Height);
					convPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / screenViewport.Width,
						1.0f / screenViewport.Height);
					auto currPassCB = mCurrFrameResource->PassCB.get();
					// Cube map pass cbuffers are stored in elements 1-6.
					currPassCB->CopyData(1 + Init + 6 + 6 * i, convPassCB);

					D3D12_CPU_DESCRIPTOR_HANDLE iblRtv = mConvAndPrefilterCubeRTs[i]->Rtv(Init);
					mCubeMapConvPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource,
						1 + Init + 6 + 6 * i, &iblRtv, 1);
					Init++;
				}
			}
			for (size_t i = 0; i < SkyCubeConvFilterNum; i++)
			{
				mCommandList->ResourceBarrier(1,
					&CD3DX12_RESOURCE_BARRIER::Transition(mConvAndPrefilterCubeRTs[i]->Resource(),
						D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
				mConvAndPrefilterSkyCubeResource[i].resize(1);
				mConvAndPrefilterSkyCubeResource[i][0] = mConvAndPrefilterCubeRTs[i]->Resource();
			}

			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(GetResourceManager()->GetOrAllocDescriptorHeap(SRVs::DeferredSrvName)->GetCPUtDescriptorHandle(0));
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			md3dDevice->CreateShaderResourceView(mBrdfLutRT2D->mResource.Get(), &srvDesc, hDescriptor);
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			for (size_t i = 0; i < SkyCubeMips; i++)
			{
				hDescriptor.Offset(1, mCbvSrvDescriptorSize);
				md3dDevice->CreateShaderResourceView(mSkyCubeRT[i]->Resource(), &srvDesc, hDescriptor);
			}
			srvDesc.Texture2D.MipLevels = 1;
			for (int i = 0; i < SkyCubeConvFilterNum; i++)
			{
				hDescriptor.Offset(1, mCbvSrvDescriptorSize);
				md3dDevice->CreateShaderResourceView(mConvAndPrefilterCubeRTs[i]->Resource(),
					&srvDesc, hDescriptor);
			}
			mDrawSkyPass->Initialize(mSkyCubeResource);
		}
#endif
#endif
		// Execute the initialization commands.
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsList0[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsList0), cmdsList0);
		// Wait until initialization is complete
		FlushCommandQueue();
		return true;
	}

	void SDxRendererGM::OnResize(UINT Width, UINT Height)
	{
		SakuraD3D12GraphicsManager::OnResize(Width, Height);
		mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
#if defined(Sakura_Defferred)
		for (int i = 0; i < GBufferRTNum; i++)
		{
			GBufferRTs[i]->OnResize(md3dDevice.Get(), Width, Height);
		}
		GetFrameGraph()->
			GetNamedRenderPass(ConsistingPasses::GBufferPassName)->Initialize(mGBufferSrvResources);
		auto GBufferAlbedo = GBufferRTs[0];
		auto GBufferNormal = GBufferRTs[1];
		auto GBufferWPos = GBufferRTs[2];
		auto GBufferRMO = GBufferRTs[3];
		mDeferredSrvResources.resize(5 + _countof(mConvAndPrefilterCubeRTs));
		mDeferredSrvResources[0] = GBufferAlbedo->mResource.Get();
		mDeferredSrvResources[1] = GBufferNormal->mResource.Get();
		mDeferredSrvResources[2] = GBufferWPos->mResource.Get();
		mDeferredSrvResources[3] = GBufferRMO->mResource.Get();
		mDeferredSrvResources[4] = mBrdfLutRT2D->mResource.Get();
		for (size_t i = 0; i < _countof(mConvAndPrefilterCubeRTs); i++)
			mDeferredSrvResources[5 + i] = mConvAndPrefilterCubeRTs[i]->Resource();
		GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::DeferredPassName)->Initialize(mDeferredSrvResources);
#if defined(Sakura_GBUFFER_DEBUG)
		GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::GBufferDebugPassName)->Initialize(mDeferredSrvResources);
#endif
#endif
#if defined(Sakura_SSAO)
		std::vector<ComPtr<ID3D12DescriptorHeap>> ssaoDescHeaps;
		mSsaoSrvResources[0] = GBufferRTs[1]->mResource.Get();
		mSsaoSrvResources[1] = mDepthStencilBuffer.Get();
		auto mSsaoPass = GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::SsaoPassName);
		mSsaoPass->Initialize(mSsaoSrvResources);
#endif
#if defined(Sakura_MotionVector)
		mMotionVectorRT->OnResize(md3dDevice.Get(), Width, Height);
#endif
#if defined(Sakura_TAA)
		for (int i = 0; i < TAARtvsNum; i++)
		{
			mTaaRTs[i]->OnResize(md3dDevice.Get(), Width, Height);
		}
		mTaaResources.resize(5);
		mTaaResources[0] = mTaaRTs[1]->mResource.Get(); // History Texture
		mTaaResources[1] = mTaaRTs[2]->mResource.Get();
		mTaaResources[2] = mTaaRTs[0]->mResource.Get(); // Inputs
		mTaaResources[3] = mMotionVectorRT->mResource.Get();  
		mTaaResources[4] = mDepthStencilBuffer.Get();
		GetFrameGraph()->
			GetNamedRenderPass(ConsistingPasses::TaaPassName)->Initialize(mTaaResources);
#endif
	}

	void SDxRendererGM::Draw()
	{
		auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
		ThrowIfFailed(cmdListAlloc->Reset());
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), nullptr));
		// Indicate a state transition on the resource usage.
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Black, 0, nullptr);

		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);
#if defined(Sakura_Defferred)
#if defined(REVERSE_Z)
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0.f, 0.f, nullptr);
#else
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0.f, 0.f, nullptr);
#endif
		// Early-Z and Motion Vector
#if defined(Sakura_MotionVector)
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mMotionVectorRT->mResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
		mMotionVectorRT->ClearRenderTarget(mCommandList.Get());
		D3D12_CPU_DESCRIPTOR_HANDLE mvRtv[1] = { mMotionVectorRT->mSRtv.hCpu };
		auto mMotionVectorPass = GetFrameGraph()->
			GetNamedRenderPass(ConsistingPasses::MotionVectorPassName);
		mMotionVectorPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Opaque]);
		mMotionVectorPass->Draw(mCommandList.Get(), &DepthStencilView(), mCurrFrameResource, mvRtv, 1);
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mMotionVectorRT->mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
#endif
		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[GBufferRTNum];
		for (int i = 0; i < GBufferRTNum; i++)
		{
			rtvs[i] = GBufferRTs[i]->mSRtv.hCpu;
			mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[i]->mResource.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
			GBufferRTs[i]->ClearRenderTarget(mCommandList.Get());
		}
		GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::GBufferPassName)
			->PushRenderItems(mRenderLayers[SRenderLayers::E_Opaque]);
		GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::GBufferPassName)
			->Draw(mCommandList.Get(), &DepthStencilView(), mCurrFrameResource, rtvs, GBufferRTNum);
		// Change back to GENERIC_READ so we can read the texture in a shader.
		for (int i = 0; i < GBufferRTNum; i++)
			mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[i]->mResource.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	#if defined(Sakura_SSAO)
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[1]->mResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
		auto mSsaoPass = GetFrameGraph()->
			GetNamedRenderPass(ConsistingPasses::SsaoPassName);
		mSsaoPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource, &GBufferRTs[1]->mSRtv.hCpu, 1);
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[1]->mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	#endif
#endif

		D3D12_CPU_DESCRIPTOR_HANDLE rtv_thisPhase = CurrentBackBufferView();
#if defined(Sakura_TAA)
		rtv_thisPhase = mTaaRTs[0]->mSRtv.hCpu;
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mTaaRTs[0]->mResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
		mCommandList->ClearRenderTargetView(mTaaRTs[0]->mSRtv.hCpu, mTaaRTs[0]->mProperties.mClearColor, 0, nullptr);
#endif
#if defined(Sakura_Defferred)
	{
		mCommandList->OMSetRenderTargets(1, &rtv_thisPhase, true, nullptr);
		GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::DeferredPassName)
			->Draw(mCommandList.Get(), nullptr, mCurrFrameResource, &rtv_thisPhase, 1);
	}
#endif
		
#if defined(Sakura_IBL)
		GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::SkySpherePassName)
			->Draw(mCommandList.Get(), &DepthStencilView(), mCurrFrameResource, &rtv_thisPhase, 1);
#endif
		rtv_thisPhase = CurrentBackBufferView();
#if defined(Sakura_TAA)
		static int mTaaChain = 0;
		auto mTaaPass = GetFrameGraph()->
			GetNamedRenderPass<STaaPass>(ConsistingPasses::TaaPassName);
		mTaaPass->ResourceIndex = mTaaChain;
		mTaaChain = (mTaaChain + 1) % 2;
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mTaaRTs[mTaaChain + 1]->mResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
		D3D12_CPU_DESCRIPTOR_HANDLE rtv_Taa[1];
		rtv_Taa[0] = mTaaRTs[mTaaChain + 1]->mSRtv.hCpu;
		mTaaPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource, rtv_Taa, 1);
		rtv_Taa[0] = rtv_thisPhase;
		mTaaPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource, rtv_Taa, 1);
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mTaaRTs[0]->mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mTaaRTs[mTaaChain + 1]->mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
#endif
		// Debug
#if defined(Sakura_GBUFFER_DEBUG) 
		GetFrameGraph()->
			GetNamedRenderPass(ConsistingPasses::GBufferDebugPassName)
			->Draw(mCommandList.Get(), &DepthStencilView(), mCurrFrameResource, &CurrentBackBufferView(), 1);
#endif
		// Indicate a state transition on the resource usage.
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		// Done recording commands.
		ThrowIfFailed(mCommandList->Close());
		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Advance the fence value to mark commands up to this fence point.
		mCurrFrameResource->Fence = ++mFence->currentFence;

		mCommandQueue->Signal(mFence->fence.Get(), mFence->currentFence);
		ThrowIfFailed(mSwapChain->Present(0, 0));
		mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
	}
	void SDxRendererGM::Finalize()
	{
	}
	void SDxRendererGM::Tick(double deltaTime)
	{
		OnKeyDown(deltaTime);
		//mCamera.UpdateViewMatrix();
		//mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		// Cycle through the circular frame resource array.
		mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
		mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

		// Has the GPU finished processing the commands of the current frame resource? 
		// If not, wait until the GPU has completed commands up to this fence point
		if (mCurrFrameResource->Fence != 0 && mFence->fence->GetCompletedValue() < mCurrFrameResource->Fence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(mFence->fence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
		UpdateObjectCBs();
		UpdateMaterialCBs();
		UpdateMainPassCB();
#if defined(Sakura_SSAO)
		UpdateSsaoPassCB();
#endif
	}

	void SDxRendererGM::OnMouseDown(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}

	void SDxRendererGM::OnMouseMove(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y)
	{
		if ((abs(x - mLastMousePos.x) > 100) | (abs(y - mLastMousePos.y) > 100))
		{
			mLastMousePos.x = x;
			mLastMousePos.y = y;
			return;
		}
		if ((btnState & SAKURA_INPUT_MOUSE_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

			mCamera.Pitch(dy);
			mCamera.RotateY(dx);
		}
		else if ((btnState & SAKURA_INPUT_MOUSE_RBUTTON) != 0)
		{
			// Make each pixel correspond to 0.2 unit in the scene.
			float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
			float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);


			mCamera.Walk(dx * -10.f);
		}
		mLastMousePos.x = x;
		mLastMousePos.y = y;

	}
	void SDxRendererGM::OnMouseUp(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y)
	{
	}
	float* SDxRendererGM::CaputureBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		ID3D12Resource* resourceToRead, size_t outChannels /*= 4*/)
	{
		auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;
		FlushCommandQueue();
		ThrowIfFailed(cmdListAlloc->Reset());
		ThrowIfFailed(cmdList->Reset(cmdListAlloc.Get(), nullptr));

		float outputBufferSize = outChannels * sizeof(float) * resourceToRead->GetDesc().Height * resourceToRead->GetDesc().Width;

		// The readback buffer (created below) is on a readback heap, so that the CPU can access it.
		D3D12_HEAP_PROPERTIES readbackHeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK) };
		D3D12_RESOURCE_DESC readbackBufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(outputBufferSize) };
		ComPtr<::ID3D12Resource> readbackBuffer;
		ThrowIfFailed(device->CreateCommittedResource(
			&readbackHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&readbackBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, IID_PPV_ARGS(&readbackBuffer)));

		{
			D3D12_RESOURCE_BARRIER outputBufferResourceBarrier
			{
				CD3DX12_RESOURCE_BARRIER::Transition(
					resourceToRead,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					D3D12_RESOURCE_STATE_COPY_SOURCE)
			};
			cmdList->ResourceBarrier(1, &outputBufferResourceBarrier);
		}

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT  fp;
		UINT nrow;
		UINT64 rowsize, size;
		device->GetCopyableFootprints(&resourceToRead->GetDesc(), 0, 1, 0, &fp, &nrow, &rowsize, &size);

		D3D12_TEXTURE_COPY_LOCATION td;
		td.pResource = readbackBuffer.Get();
		td.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		td.PlacedFootprint = fp;
		
		D3D12_TEXTURE_COPY_LOCATION ts;
		ts.pResource = resourceToRead;
		ts.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		ts.SubresourceIndex = 0;
		cmdList->CopyTextureRegion(&td, 0, 0, 0, &ts, nullptr);

		{
			D3D12_RESOURCE_BARRIER outputBufferResourceBarrier
			{
				CD3DX12_RESOURCE_BARRIER::Transition(
					resourceToRead,
					D3D12_RESOURCE_STATE_COPY_SOURCE,
					D3D12_RESOURCE_STATE_GENERIC_READ)
			};
			cmdList->ResourceBarrier(1, &outputBufferResourceBarrier);
		}

		// Code goes here to close, execute (and optionally reset) the command list, and also
		// to use a fence to wait for the command queue.
		cmdList->Close();
		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { cmdList };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
		FlushCommandQueue();
		// The code below assumes that the GPU wrote FLOATs to the buffer.
		D3D12_RANGE readbackBufferRange{ 0, outputBufferSize };
		FLOAT* pReadbackBufferData{};
		ThrowIfFailed(
			readbackBuffer->Map
			(
				0,
				&readbackBufferRange,
				reinterpret_cast<void**>(&pReadbackBufferData)
			)
		);
		const char** err = nullptr;
		SaveEXR(pReadbackBufferData, resourceToRead->GetDesc().Width, resourceToRead->GetDesc().Height,
			4, 0, "Capture.exr", err);
		D3D12_RANGE emptyRange{ 0, 0 };
		readbackBuffer->Unmap
		(
			0,
			&emptyRange
		);
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		ThrowIfFailed(cmdListAlloc->Reset());

		// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
		// Reusing the command list reuses memory.
		ThrowIfFailed(cmdList->Reset(cmdListAlloc.Get(), nullptr));
		// Code goes here to close, execute (and optionally reset) the command list, and also
		// to use a fence to wait for the command queue.
		cmdList->Close();
		return pReadbackBufferData;
	}

	void SDxRendererGM::OnKeyDown(double deltaTime)
	{
		auto dt = deltaTime;

		if (GetAsyncKeyState('W') & 0x8000)
			mCamera.Walk(100.0f * dt);
		if (GetAsyncKeyState('S') & 0x8000)
			mCamera.Walk(-100.0f * dt);
		if (GetAsyncKeyState('A') & 0x8000)
			mCamera.Strafe(-100.0f * dt);
		if (GetAsyncKeyState('D') & 0x8000)
			mCamera.Strafe(100.0f * dt);
		if (GetAsyncKeyState('F') & 0x8000)
			mCamera.SetPosition(0.0f, 0.0f, -100.0f);
		static bool Cap = false;
		if (GetAsyncKeyState('P') & 0x8000 && Cap)
		{
			//CaputureBuffer(md3dDevice.Get(), mCommandList.Get(),
			//	mBrdfLutRT2D->mResource.Get(),
			//	4);
			//Cap = false;
		}else Cap = true;

		mCamera.UpdateViewMatrix();
	}

	void SDxRendererGM::UpdateObjectCBs()
	{
		auto CurrObjectCB = mCurrFrameResource->ObjectCB.get();
		for (auto& e : mAllRitems)
		{
			// Only update the cbuffer data if the constants have changed.
			// This needs to be tracked per frame resource
			if (e->NumFramesDirty > 0)
			{
				XMMATRIX world = XMLoadFloat4x4(&e->World);
				XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);
				XMMATRIX prevWorld = XMLoadFloat4x4(&e->PrevWorld);

				SRenderMeshConstants objConstants; 
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
				XMStoreFloat4x4(&objConstants.PrevWorld, XMMatrixTranspose(prevWorld));

				CurrObjectCB->CopyData(e->ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				e->NumFramesDirty--;
			}
		}
	}

	void SDxRendererGM::UpdateMaterialCBs()
	{
		auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
		for (auto& e : mMaterials)
		{
			// Only update the cbuffer data if the constants have changed. If the cbuffer
			// data changes, it needs to be updated for each FrameResource.
			OpaqueMaterial* mat = e.second;
			if (mat->NumFramesDirty > 0)
			{
				PBRMaterialConstants matConstants;
				matConstants = mat->MatConstants;

				currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

				// Next FrameResource need to be updated too.
				mat->NumFramesDirty--;
			}
		}
	}
	// 
	void SDxRendererGM::UpdateMainPassCB()
	{
		static int mFrameCount = 0;
		mFrameCount = (mFrameCount + 1) % TAA_SAMPLE_COUNT;
		XMMATRIX proj = mCamera.GetProj();
		XMMATRIX view = mCamera.GetView();
		mMainPassCB.PrevViewProj = mMainPassCB.UnjitteredViewProj;
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
#if defined(Sakura_TAA)
		XMStoreFloat4x4(&mMainPassCB.UnjitteredViewProj, XMMatrixTranspose(viewProj));
		double JitterX = SGraphics::Halton_2[mFrameCount] / (double)mGraphicsConfs->clientWidth * (double)TAA_JITTER_DISTANCE;
		double JitterY = SGraphics::Halton_3[mFrameCount] / (double)mGraphicsConfs->clientHeight * (double)TAA_JITTER_DISTANCE;
		proj.r[2].m128_f32[0] += (float)JitterX;
		proj.r[2].m128_f32[1] += (float)JitterY;
		mMainPassCB.AddOnMsg = mFrameCount;
		mMainPassCB.Jitter.x = JitterX;
		mMainPassCB.Jitter.y = -JitterY;
#endif
		viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
		mMainPassCB.EyePosW = mCamera.GetPosition3f();
		mMainPassCB.RenderTargetSize = XMFLOAT2((float)mGraphicsConfs->clientWidth, (float)mGraphicsConfs->clientHeight);
		mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mGraphicsConfs->clientWidth, 1.0f / mGraphicsConfs->clientHeight);
#if defined(REVERSE_Z)
		mMainPassCB.FarZ = 1.0f;
		mMainPassCB.NearZ = 1000.0f;
#else
		mMainPassCB.NearZ = 1.0f;
		mMainPassCB.FarZ = 1000.0f;
#endif
		mMainPassCB.TotalTime = 1;
		mMainPassCB.DeltaTime = 1;

		mMainPassCB.AmbientLight = { 0.f, 0.f, 0.f, 0.f };
		mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		mMainPassCB.Lights[0].Strength = { 1.000000000f, 1.000000000f, 0.878431439f };
		mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
		mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

		auto currPassCB = mCurrFrameResource->PassCB.get();
		currPassCB->CopyData(0, mMainPassCB);
	}

	void SDxRendererGM::UpdateSsaoPassCB()
	{
		XMMATRIX proj = mCamera.GetProj();

		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		mSsaoCB.View = mMainPassCB.View;
		mSsaoCB.Proj = mMainPassCB.Proj;
		mSsaoCB.InvProj = mMainPassCB.InvProj;
		XMStoreFloat4x4(&mSsaoCB.ProjTex, XMMatrixTranspose(proj * T));
		
		SsaoPass* mSsaoPas = (SsaoPass*)(GetFrameGraph()->GetNamedRenderPass(ConsistingPasses::SsaoPassName));
		mSsaoPas->GetOffsetVectors(mSsaoCB.OffsetVectors);
		auto blurWeights = mSsaoPas->CalcGaussWeights(2.5f);

		mSsaoCB.InvRenderTargetSize = XMFLOAT2(1.0f / (mGraphicsConfs->clientWidth), 1.0f / (mGraphicsConfs->clientHeight));

		// Coordinates given in view space.
		mSsaoCB.OcclusionRadius = 0.5f;
		mSsaoCB.OcclusionFadeStart = 0.2f;
		mSsaoCB.OcclusionFadeEnd = 1.0f;
		mSsaoCB.SurfaceEpsilon = 0.05f;

		auto currSsaoCB = mCurrFrameResource->SsaoCB.get();
		currSsaoCB->CopyData(0, mSsaoCB);
	}

	void SDxRendererGM::LoadTextures()
	{
		for (int i = 0; i < Textures::texNames.size(); ++i)
		{
			pGraphicsResourceManager->LoadTextures(Textures::texFilenames[i], Textures::texNames[i]);
		}
	}

	void SDxRendererGM::BuildDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = GBufferResourceSrv;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		((SDxResourceManager*)pGraphicsResourceManager.get())
			->GetOrAllocDescriptorHeap(SRVs::GBufferSrvName, mDeviceInformation->cbvSrvUavDescriptorSize, srvHeapDesc);

		srvHeapDesc.NumDescriptors = 1;
		((SDxResourceManager*)pGraphicsResourceManager.get())
			->GetOrAllocDescriptorHeap(SRVs::CaptureSrvName, mDeviceInformation->cbvSrvUavDescriptorSize, srvHeapDesc);

		srvHeapDesc.NumDescriptors = GBufferRTNum + GBufferSrvStartAt + LUTNum;
		((SDxResourceManager*)pGraphicsResourceManager.get())
			->GetOrAllocDescriptorHeap(SRVs::DeferredSrvName, mDeviceInformation->cbvSrvUavDescriptorSize, srvHeapDesc);

		srvHeapDesc.NumDescriptors = 10;
		((SDxResourceManager*)pGraphicsResourceManager.get())
			->GetOrAllocDescriptorHeap(SRVs::ScreenEfxSrvName, mDeviceInformation->cbvSrvUavDescriptorSize, srvHeapDesc);
	}

	void SDxRendererGM::BindPassResources()
	{
		auto DiffTex = ((SD3DTexture*)(GetResourceManager()->GetTexture("DiffTex")))->Resource;
		auto RoughTex = ((SD3DTexture*)(GetResourceManager()->GetTexture("RoughTex")))->Resource;
		auto SpecTex = ((SD3DTexture*)(GetResourceManager()->GetTexture("SpecTex")))->Resource;
		auto NormTex = ((SD3DTexture*)(GetResourceManager()->GetTexture("NormalTex")))->Resource;
		auto SkyTex = ((SD3DTexture*)(GetResourceManager()->GetTexture("SkyCubeMap")))->Resource;
		
		mSkyCubeResource.resize(1);
		mSkyCubeResource[0] = (SkyTex.Get());
		mGBufferSrvResources.resize(4);
		mGBufferSrvResources[0] = (DiffTex.Get());
		mGBufferSrvResources[1] = (RoughTex.Get());
		mGBufferSrvResources[2] = (SpecTex.Get());
		mGBufferSrvResources[3] = (NormTex.Get());
		mSsaoSrvResources.resize(2);
		mSsaoSrvResources[0] = (GBufferRTs[1]->mResource.Get());
		mSsaoSrvResources[1] = (mDepthStencilBuffer.Get());
	}

	void SDxRendererGM::BuildGeneratedMeshes()
	{
		GeometryGenerator geoGen;
		auto quad = geoGen.CreateQuad(0.f, 0.f, 1.f, 1.f, 0.f);

		SubmeshGeometry quadSubmesh;
		quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
		quadSubmesh.StartIndexLocation = 0;
		quadSubmesh.BaseVertexLocation = 0;

		std::vector<ScreenQuadVertex> vertices(quadSubmesh.IndexCount);
		for (int i = 0; i < quad.Vertices.size(); ++i)
		{
			vertices[i].Pos = quad.Vertices[i].Position;
			vertices[i].TexC = quad.Vertices[i].TexC;
		}
		std::vector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

		auto geo = std::make_unique<Dx12MeshGeometry>();
		geo->Name = "ScreenQuad";

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(ScreenQuadVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		// Create StandardVertex Buffer Blob
		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
		
		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
		geo->VertexByteStride = sizeof(ScreenQuadVertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexFormat = DXGI_FORMAT_R16_UINT;
		geo->IndexBufferByteSize = ibByteSize;

		geo->DrawArgs["ScreenQuad"] = quadSubmesh;
		mGeometries["ScreenQuad"] = std::move(geo);
		
#if defined(Sakura_GBUFFER_DEBUG) 
		{
		quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
		quadSubmesh.StartIndexLocation = 0;
		quadSubmesh.BaseVertexLocation = 0;

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(ScreenQuadDebugVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		for (int j = 0; j < 6; j++)
		{
			auto geo = std::make_unique<Dx12MeshGeometry>();
			std::string Name = "DebugScreenQuad" + std::to_string(j);
			mGeometries[Name] = std::move(geo);

			std::vector<ScreenQuadDebugVertex> vertices(quadSubmesh.IndexCount);
			for (int i = 0; i < quad.Vertices.size(); ++i)
			{
				vertices[i].Pos = quad.Vertices[i].Position;
				vertices[i].TexC = quad.Vertices[i].TexC;
				vertices[i].Type = j;
			}
			std::vector<std::uint16_t> indices;
			indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

			// Create StandardVertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeometries[Name]->VertexBufferCPU));
			CopyMemory(mGeometries[Name]->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeometries[Name]->IndexBufferCPU));
			CopyMemory(mGeometries[Name]->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			mGeometries[Name]->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, mGeometries[Name]->VertexBufferUploader);
			mGeometries[Name]->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, mGeometries[Name]->IndexBufferUploader);
			mGeometries[Name]->VertexByteStride = sizeof(ScreenQuadDebugVertex);
			mGeometries[Name]->VertexBufferByteSize = vbByteSize;
			mGeometries[Name]->IndexFormat = DXGI_FORMAT_R16_UINT;
			mGeometries[Name]->IndexBufferByteSize = ibByteSize;
			mGeometries[Name]->DrawArgs[Name] = quadSubmesh;
		}
		}
#endif
		
#if defined(Sakura_IBL)
		{
			auto sphere = geoGen.CreateSphere(0.5f, 120, 120);
			SubmeshGeometry quadSubmesh;
			quadSubmesh.IndexCount = (UINT)sphere.Indices32.size();
			quadSubmesh.StartIndexLocation = 0;
			quadSubmesh.BaseVertexLocation = 0;

			std::vector<StandardVertex> vertices(quadSubmesh.IndexCount);
			for (int i = 0; i < sphere.Vertices.size(); ++i)
			{
				vertices[i].Pos = sphere.Vertices[i].Position;
				vertices[i].TexC = sphere.Vertices[i].TexC;
				vertices[i].Normal = sphere.Vertices[i].Normal;
				vertices[i].TexC = sphere.Vertices[i].TexC;
				vertices[i].Tangent = sphere.Vertices[i].TangentU;
			}
			std::vector<std::uint16_t> indices;
			indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));

			auto geo = std::make_unique<Dx12MeshGeometry>();
			geo->Name = "SkySphere";

			const UINT vbByteSize = (UINT)vertices.size() * sizeof(StandardVertex);
			const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

			// Create StandardVertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
			CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
			geo->VertexByteStride = sizeof(StandardVertex);
			geo->VertexBufferByteSize = vbByteSize;
			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
			geo->IndexBufferByteSize = ibByteSize;

			geo->DrawArgs[geo->Name] = quadSubmesh;
			mGeometries[geo->Name] = std::move(geo);
		}
#endif
#if defined(Sakura_IBL_HDR_INPUT)
		{
			auto cube = geoGen.CreateBox(1.f, 1.f, 1.f, 1);
			SubmeshGeometry quadSubmesh;
			quadSubmesh.IndexCount = (UINT)cube.Indices32.size();
			quadSubmesh.StartIndexLocation = 0;
			quadSubmesh.BaseVertexLocation = 0;

			std::vector<StandardVertex> vertices(quadSubmesh.IndexCount);
			for (int i = 0; i < cube.Vertices.size(); ++i)
			{
				vertices[i].Pos = cube.Vertices[i].Position;
				vertices[i].TexC = cube.Vertices[i].TexC;
				vertices[i].Normal = cube.Vertices[i].Normal;
				vertices[i].TexC = cube.Vertices[i].TexC;
				vertices[i].Tangent = cube.Vertices[i].TangentU;
			}
			std::vector<std::uint16_t> indices;
			indices.insert(indices.end(), std::begin(cube.GetIndices16()), std::end(cube.GetIndices16()));

			auto geo = std::make_unique<Dx12MeshGeometry>();
			geo->Name = "HDRCube";

			const UINT vbByteSize = (UINT)vertices.size() * sizeof(StandardVertex);
			const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

			// Create StandardVertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
			CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
			geo->VertexByteStride = sizeof(StandardVertex);
			geo->VertexBufferByteSize = vbByteSize;
			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
			geo->IndexBufferByteSize = ibByteSize;

			geo->DrawArgs[geo->Name] = quadSubmesh;
			mGeometries[geo->Name] = std::move(geo);
		}
#endif
	}

	void SDxRendererGM::BuildGeometry()
	{
		mGeometries["skullGeo"] = std::move(MeshImporter::ImportMesh(md3dDevice.Get(), mCommandList.Get(), Meshes::CurrPath, ESupportFileForm::ASSIMP_SUPPORTFILE));
		BuildGeneratedMeshes();
	}

	void SDxRendererGM::BuildFrameResources()
	{
		for (int i = 0; i < gNumFrameResources; i++)
		{
			mFrameResources.push_back(std::make_unique<SFrameResource>(md3dDevice.Get(),
				1 + 6 + SkyCubeConvFilterNum * 6, (UINT)mAllRitems.size() * 50, (UINT)mMaterials.size() + 200));
		}
	}

	void SDxRendererGM::BuildMaterials()
	{
		int MatCBInd = 0;
		auto bricks0 = new OpaqueMaterial();
		bricks0->Name = "bricks0";
		bricks0->MatCBIndex = MatCBInd++;
		bricks0->MatConstants.DiffuseSrvHeapIndex = 0;
		bricks0->MatConstants.RMOSrvHeapIndex = 1;
		bricks0->MatConstants.SpecularSrvHeapIndex = 2;
		bricks0->MatConstants.NormalSrvHeapIndex = 3;
		bricks0->MatConstants.BaseColor = XMFLOAT3(Colors::White);
		bricks0->MatConstants.Roughness = 1.f;
		GBufferResourceSrv += 4;
		mMaterials["bricks0"] = bricks0;
	}
	
	void SDxRendererGM::BuildRenderItems()
	{
#if defined(Sakura_Full_Effects)
		auto opaqueRitem = std::make_unique<SDxRenderItem>();
		XMStoreFloat4x4(&opaqueRitem->World, XMMatrixScaling(.2f, .2f, .2f) * 
		XMMatrixTranslation(0.f, -6.f, 0.f) * XMMatrixRotationY(155));
		opaqueRitem->PrevWorld = opaqueRitem->World;
		opaqueRitem->TexTransform = MathHelper::Identity4x4();
		opaqueRitem->ObjCBIndex = CBIndex++;
		opaqueRitem->Mat = mMaterials["bricks0"];
		opaqueRitem->Geo = mGeometries["skullGeo"].get();
		opaqueRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		opaqueRitem->IndexCount = opaqueRitem->Geo->DrawArgs["mesh"].IndexCount;
		opaqueRitem->StartIndexLocation = opaqueRitem->Geo->DrawArgs["mesh"].StartIndexLocation;
		opaqueRitem->BaseVertexLocation = opaqueRitem->Geo->DrawArgs["mesh"].BaseVertexLocation;
		mRenderLayers[SRenderLayers::E_Opaque].push_back(opaqueRitem.get());
		mAllRitems.push_back(std::move(opaqueRitem));
#endif
		auto screenQuad = std::make_unique<SDxRenderItem>();
		screenQuad->World = MathHelper::Identity4x4();
		screenQuad->TexTransform = MathHelper::Identity4x4();
		screenQuad->ObjCBIndex = CBIndex++;
		screenQuad->Mat = mMaterials["bricks0"];
		screenQuad->Geo = mGeometries["ScreenQuad"].get();
		screenQuad->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		screenQuad->IndexCount = screenQuad->Geo->DrawArgs["ScreenQuad"].IndexCount;
		screenQuad->StartIndexLocation = screenQuad->Geo->DrawArgs["ScreenQuad"].StartIndexLocation;
		screenQuad->BaseVertexLocation = screenQuad->Geo->DrawArgs["ScreenQuad"].BaseVertexLocation;

		mRenderLayers[SRenderLayers::E_ScreenQuad].push_back(screenQuad.get());
		mAllRitems.push_back(std::move(screenQuad));

#if defined(Sakura_IBL)
		auto skyRitem = std::make_unique<SDxRenderItem>();
		XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.f, 5000.f, 5000.f));
		skyRitem->TexTransform = MathHelper::Identity4x4();
		skyRitem->ObjCBIndex = CBIndex++;
		skyRitem->Mat = mMaterials["bricks0"];
		skyRitem->Geo = mGeometries["SkySphere"].get();
		skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		skyRitem->IndexCount = skyRitem->Geo->DrawArgs["SkySphere"].IndexCount;
		skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["SkySphere"].StartIndexLocation;
		skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["SkySphere"].BaseVertexLocation;

		mRenderLayers[SRenderLayers::E_SKY].push_back(skyRitem.get());
		mAllRitems.push_back(std::move(skyRitem));
#endif

#if defined(Sakura_IBL_HDR_INPUT)
		auto boxRitem = std::make_unique<SDxRenderItem>();
		XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(1000.f, 1000.f, 1000.f));
		boxRitem->TexTransform = MathHelper::Identity4x4();
		boxRitem->ObjCBIndex = CBIndex++;
		boxRitem->Mat = mMaterials["bricks0"];
		boxRitem->Geo = mGeometries["HDRCube"].get();
		boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		boxRitem->IndexCount = boxRitem->Geo->DrawArgs["HDRCube"].IndexCount;
		boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["HDRCube"].StartIndexLocation;
		boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["HDRCube"].BaseVertexLocation;

		mRenderLayers[SRenderLayers::E_Cube].push_back(boxRitem.get());
		mAllRitems.push_back(std::move(boxRitem));
#endif

#if defined(Sakura_GBUFFER_DEBUG) 
		for (int i = 0; i < 6; i++)
		{
			std::string Name = "DebugScreenQuad" + std::to_string(i);
			screenQuad = std::make_unique<SDxRenderItem>();
			screenQuad->World = MathHelper::Identity4x4();
			screenQuad->TexTransform = MathHelper::Identity4x4();
			screenQuad->ObjCBIndex = CBIndex++;
			screenQuad->Mat = mMaterials["bricks0"];
			screenQuad->Geo = mGeometries[Name].get();
			screenQuad->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			screenQuad->IndexCount = screenQuad->Geo->DrawArgs[Name].IndexCount;
			screenQuad->StartIndexLocation = screenQuad->Geo->DrawArgs[Name].StartIndexLocation;
			screenQuad->BaseVertexLocation = screenQuad->Geo->DrawArgs[Name].BaseVertexLocation;

			mRenderLayers[SRenderLayers::E_GBufferDebug].push_back(screenQuad.get());
			mAllRitems.push_back(std::move(screenQuad));
		}
#endif
	}


	void SDxRendererGM::CreateRtvAndDsvDescriptorHeaps()
	{
		//Create render target view descriptor for swap chain buffers
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		((SDxResourceManager*)(pGraphicsResourceManager.get()))
			->GetOrAllocDescriptorHeap("DefaultRtv", mDeviceInformation->rtvDescriptorSize, rtvHeapDesc);

		rtvHeapDesc.NumDescriptors =  GBufferRTNum + 
			6 * SkyCubeMips + 6 * SkyCubeConvFilterNum + 100;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		((SDxResourceManager*)(pGraphicsResourceManager.get()))
			->GetOrAllocDescriptorHeap(RTVs::DeferredRtvName, mDeviceInformation->rtvDescriptorSize, rtvHeapDesc);

		rtvHeapDesc.NumDescriptors = 1;
		((SDxResourceManager*)(pGraphicsResourceManager.get()))
			->GetOrAllocDescriptorHeap(RTVs::CaptureRtvName, mDeviceInformation->rtvDescriptorSize, rtvHeapDesc);

		rtvHeapDesc.NumDescriptors = 10;
		((SDxResourceManager*)(pGraphicsResourceManager.get()))
			->GetOrAllocDescriptorHeap(RTVs::ScreenEfxRtvName, mDeviceInformation->rtvDescriptorSize, rtvHeapDesc);

		//Create depth/stencil view descriptor 
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
			&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
	}

	void SDxRendererGM::BuildCubeFaceCamera(float x, float y, float z)
	{
		// Generate the cube map about the given position.
		XMFLOAT3 center(x, y, z);
		XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);
		// Look along each coordinate axis.
		XMFLOAT3 targets[6] =
		{
			XMFLOAT3(x + 1.0f, y, z), // +X
			XMFLOAT3(x - 1.0f, y, z), // -X			
			XMFLOAT3(x, y + 1.0f, z), // +Y
			XMFLOAT3(x, y - 1.0f, z), // -Y
			XMFLOAT3(x, y, z + 1.0f), // +Z
			XMFLOAT3(x, y, z - 1.0f),  // -Z
			
		};
		// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
		// are looking down +Y or -Y, so we need a different "up" vector.
		XMFLOAT3 ups[6] =
		{
			XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
			XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
			XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y		
			XMFLOAT3(0.0f, 0.0f, +1.0f),  // -Y	
			XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
			XMFLOAT3(0.0f, 1.0f, 0.0f),	 // -Z				
			
		};
		for (int i = 0; i < 6; ++i)
		{
			mCubeMapCamera[i].LookAt(center, targets[i], ups[i]);
			mCubeMapCamera[i].SetLens(0.5f * XM_PI, 1.0f, 0.1f, 1000.0f);
			mCubeMapCamera[i].UpdateViewMatrix();
		}
	}
}