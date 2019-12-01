#include <minwindef.h>
#include "SDxRendererGM.h"

// Passes
//#define Sakura_Full_Effects
#define Sakura_Debug_PureColor

#define Sakura_Defferred 
#define Sakura_SSAO 
#define Sakura_SSAO_DEBUG 
#define Sakura_IBL
#define Sakura_IBL_HDR_INPUT

#include "Pipeline/SsaoPass.hpp"
#include "Pipeline/GBufferPass.hpp"
#include "Pipeline/IBL/SkySpherePass.hpp"
#include "Pipeline/IBL/HDR2CubeMapPass.hpp"
#include "Pipeline/IBL/CubeMapConvolutionPass.hpp"

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
		BuildGeneratedMeshes();
		BuildGeometry();
		BuildMaterials();
		BuildRenderItems();
		BuildDescriptorHeaps();
		
		BuildShaderAndInputLayout();
		BuildGBufferPassShaderAndInputLayout();
		BuildDeferredShadingPassShaderAndInputLayout();

#if defined(DEBUG) || defined(_DEBUG)
		BuildGBufferDebugShaderAndInputLayout();
#endif
		// ¡ü Do not have dependency on dx12 resources
		{
#if defined(Sakura_Defferred)
			GBufferRTs = new std::shared_ptr<SRenderTarget2D>[GBufferRTNum];
			for (int i = 0; i < GBufferRTNum; i++)
			{
				//SRTProperties prop, UINT ClientWidth, UINT ClientHeight, bool ScaledByViewport = true
				SRTProperties prop(0.f, 0.f, (i == 3) ? 1.f : 0.f, 0.f);
				GBufferRTs[i] = std::make_shared<SRenderTarget2D>(prop, mClientWidth, mClientHeight, true);
				// Init RT Resource with a CPU rtv handle.
				GBufferRTs[i]->BuildRTResource(md3dDevice.Get(),
					GetRtvCPU(SwapChainBufferCount + i), 
					GetDeferredSrvCPU(GBufferSrvStartAt + i),
					GetDeferredSrvGPU(GBufferSrvStartAt + i));
			}
#endif
#if defined(Sakura_IBL)		
			UINT _sizeS = 2048;
			for (int j = 0; j < SkyCubeMips; j++)
			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE convCubeRtvHandles[6];
				mSkyCubeRT[j] = std::make_shared<SRenderTargetCube>(md3dDevice.Get(), _sizeS, _sizeS, DXGI_FORMAT_R16G16B16A16_FLOAT);
				_sizeS /= 2;
				for (int i = 0; i < 6; i++)
				{
					convCubeRtvHandles[i] = GetRtvCPU(SwapChainBufferCount + GBufferRTNum + i + j * 6);
				}
				mSkyCubeRT[j]->BuildDescriptors(
					CD3DX12_CPU_DESCRIPTOR_HANDLE(GetDeferredSrvCPU(LUTNum+ j)),
					CD3DX12_GPU_DESCRIPTOR_HANDLE(GetDeferredSrvGPU(LUTNum + j)),
					convCubeRtvHandles);
			}
			UINT size = 1024;
			CD3DX12_CPU_DESCRIPTOR_HANDLE cubeRtvHandles[6];
			for (int i = 0; i < SkyCubeConvFilterNum; i++)
			{
				if (i != 0)
				{
					mConvAndPrefilterCubeRTs[i] =
						std::make_shared<SRenderTargetCube>(md3dDevice.Get(), size, size, DXGI_FORMAT_R16G16B16A16_FLOAT);
					size = size / 2;
				}
				else
				{
					mConvAndPrefilterCubeRTs[i] =
						std::make_shared<SRenderTargetCube>(md3dDevice.Get(), 64, 64, DXGI_FORMAT_R16G16B16A16_FLOAT);
				}
				for (int j = 0; j < 6; j++)
				{
					cubeRtvHandles[j] = GetRtvCPU(SwapChainBufferCount + GBufferRTNum + SkyCubeMips * 6 + j + i * 6);
				}
				mConvAndPrefilterCubeRTs[i]->BuildDescriptors(
					GetDeferredSrvCPU(LUTNum + SkyCubeMips + i),
					GetDeferredSrvGPU(LUTNum + SkyCubeMips + i),
					cubeRtvHandles);
			}
#endif
		}
		//¡ý Do have dependency on dx12 resources
		BindPassResources();
		BuildGBufferPassDescriptorHeaps();
#if defined(Sakura_Defferred)
		BuildDeferredShadingPassDescriptorHeaps();
		mGbufferPass = std::make_shared<SGBufferPass>(md3dDevice.Get());
		mGbufferPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Opaque]);
		BuildDeferredPassRootSignature();
#endif
		BuildGBufferPassRootSignature();
		

		BuildFrameResources();
		mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
		BuildPSOs();
		


#if defined(Sakura_SSAO)
		mSsaoPass = std::make_shared<SsaoPass>(md3dDevice.Get());
		mSsaoPass->PushRenderItems(mRenderLayers[SRenderLayers::E_ScreenQuad]);
		mSsaoPass->StartUp(mCommandList.Get());
#endif

#if defined(Sakura_IBL)
		mDrawSkyPass = std::make_shared<SkySpherePass>(md3dDevice.Get());
		mDrawSkyPass->PushRenderItems(mRenderLayers[SRenderLayers::E_SKY]);
#endif

#if defined(Sakura_IBL_HDR_INPUT)
		mHDRUnpackPass = std::make_shared<SHDR2CubeMapPass>(md3dDevice.Get());
		mCubeMapConvPass = std::make_shared<SCubeMapConvPass>(md3dDevice.Get());
		mHDRUnpackPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Cube]);
		mCubeMapConvPass->PushRenderItems(mRenderLayers[SRenderLayers::E_Cube]);
		std::vector<ID3D12Resource*> mHDRResource;
		mHDRResource.push_back(mHDRTexture->Resource.Get());
		mHDRUnpackPass->Initialize(mHDRResource);
#endif
		
		// Execute the initialization commands.
		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* cmdsList[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsList), cmdsList);

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
			GBufferRTs[i]->OnResize(md3dDevice.Get(), Width, Height);
		mGbufferPass->Initialize(mGBufferSrvResources);
#endif

#if defined(Sakura_SSAO)
		mSsaoSrvResources[0] = GBufferRTs[1]->mResource.Get();
		mSsaoSrvResources[1] = mDepthStencilBuffer.Get();
		mSsaoPass->Initialize(mSsaoSrvResources);
#endif

#if defined(Sakura_IBL)
		mDrawSkyPass->Initialize(mSkyCubeResource);
#endif
	}

	void SDxRendererGM::Draw()
	{
		
		auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished execution on the GPU.
		ThrowIfFailed(cmdListAlloc->Reset());

		// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
		// Reusing the command list reuses memory.
		ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["ForwardShading"].Get()));

		// Indicate a state transition on the resource usage.
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

#if defined(Sakura_IBL_HDR_INPUT)
		static int Init = 0;
		if (Init < 6)
		{
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
					XMFLOAT2(mSkyCubeRT[i]->mWidth,
						mSkyCubeRT[i]->mHeight);
				cubeFacePassCB.InvRenderTargetSize =
					XMFLOAT2(1.0f / mSkyCubeRT[i]->mWidth,
						1.0f / mSkyCubeRT[i]->mHeight);
				screenViewport.Width = static_cast<float>(mSkyCubeRT[i]->mWidth);
				screenViewport.Height = static_cast<float>(mSkyCubeRT[i]->mHeight);
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
				screenViewport.Width = static_cast<float>(mConvAndPrefilterCubeRTs[i]->mWidth);
				screenViewport.Height = static_cast<float>(mConvAndPrefilterCubeRTs[i]->mHeight);
				screenViewport.MinDepth = 0.0f;
				screenViewport.MaxDepth = 1.0f;
				scissorRect = { 0, 0, (LONG)mConvAndPrefilterCubeRTs[i]->mWidth, 
					(LONG)mConvAndPrefilterCubeRTs[i]->mHeight };
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

			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mDeferredSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			md3dDevice->CreateShaderResourceView(mBRDF_LUT->Resource.Get(), &srvDesc, hDescriptor);
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
		mCommandList->RSSetViewports(1, &mScreenViewport);
		mCommandList->RSSetScissorRects(1, &mScissorRect);
		
#if defined(Sakura_Defferred)
		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[GBufferRTNum];
		for (int i = 0; i < GBufferRTNum; i++)
		{
			rtvs[i] = GBufferRTs[i]->mRtvCpu;
			mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[i]->mResource.Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
			GBufferRTs[i]->ClearRenderTarget(mCommandList.Get());
		}
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0.f, 0.f, nullptr);
		mGbufferPass->Draw(mCommandList.Get(), &DepthStencilView(), mCurrFrameResource, rtvs, GBufferRTNum);
		// Change back to GENERIC_READ so we can read the texture in a shader.
		for (int i = 0; i < GBufferRTNum; i++)
			mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[i]->mResource.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
		
	#if defined(Sakura_SSAO)
		//	
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[1]->mResource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
		mSsaoPass->Draw(mCommandList.Get(), nullptr, mCurrFrameResource, &GBufferRTs[1]->mRtvCpu, 1);
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GBufferRTs[1]->mResource.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
		//
	#endif
		
#else
		mCommandList->SetPipelineState(mPSOs["ForwardShading"].Get());
		mCommandList->SetGraphicsRootSignature(mRootSignatures["GBufferPass"].Get());
		// Clear the back buffer and depth buffer.
		// Set Descriptor Heaps
		ID3D12DescriptorHeap* descriptorHeaps[] = { mGBufferSrvDescriptorHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		auto passCB = mCurrFrameResource->PassCB->Resource();
		mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
#endif
		mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
#if defined(Sakura_Defferred)
	{
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);
		// Deferred Pass
		// Set Descriptor Heaps
		ID3D12DescriptorHeap* descriptorHeaps[] = { mDeferredSrvDescriptorHeap.Get() };
		mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		// Deferred shading:
		mCommandList->SetPipelineState(mPSOs["DeferredShading"].Get());
		mCommandList->SetGraphicsRootSignature(mRootSignatures["DeferredShading"].Get());

		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SRenderMeshConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto objectCB = mCurrFrameResource->ObjectCB->Resource();
		auto matCB = mCurrFrameResource->MaterialCB->Resource();

		SRenderItem* ri = mRenderLayers[SRenderLayers::E_ScreenQuad][0];

		mCommandList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		mCommandList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		mCommandList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mDeferredSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		mCommandList->SetGraphicsRootDescriptorTable(8, tex);
		tex.Offset(SkyCubeMips + LUTNum, mCbvSrvDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(7, tex);
		tex.Offset(GBufferSrvStartAt - LUTNum - SkyCubeMips, mCbvSrvDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(0, tex);
		tex.Offset(1, mCbvSrvDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(1, tex);
		tex.Offset(1, mCbvSrvDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(2, tex);
		tex.Offset(1, mCbvSrvDescriptorSize);
		mCommandList->SetGraphicsRootDescriptorTable(3, tex);
		//
		
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		mCommandList->SetGraphicsRootConstantBufferView(4, objCBAddress);
		mCommandList->SetGraphicsRootConstantBufferView(6, matCBAddress);

		mCommandList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
#else
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
		mCommandList->SetGraphicsRootDescriptorTable(0, mGBufferSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());
		DrawRenderItems(mCommandList.Get(), mRenderLayers[SRenderLayers::E_Opaque]);
#endif
		
#if defined(Sakura_IBL)
		D3D12_CPU_DESCRIPTOR_HANDLE ibl_rtv;
		ibl_rtv = CurrentBackBufferView();
		mDrawSkyPass->Draw(mCommandList.Get(), &DepthStencilView(), mCurrFrameResource, &ibl_rtv, 1);
#endif
		// Debug
#if defined(Sakura_SSAO_DEBUG) 
		mCommandList->SetPipelineState(mPSOs["GBufferDebug"].Get());
		mCommandList->SetGraphicsRootSignature(mRootSignatures["DeferredShading"].Get());
		for (auto ri : mRenderLayers[SRenderLayers::E_GBufferDebug])
		{
			mCommandList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			mCommandList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			mCommandList->IASetPrimitiveTopology(ri->PrimitiveType);
			mCommandList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
#endif
		/**/
		// Indicate a state transition on the resource usage.
		mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));


		// Done recording commands.
		ThrowIfFailed(mCommandList->Close());
		
		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		// Advance the fence value to mark commands up to this fence point.
		mCurrFrameResource->Fence = ++mCurrentFence;

		// Add an instruction to the command queue to set a new fence point.
		// Because we are on the GPU time line, the new fence point won't be 
		// set until the GPU finishes processing all the commands prior to this Signal().
		mCommandQueue->Signal(mFence.Get(), mCurrentFence);

		// Swap the back and front buffers.
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
		if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

		AnimateMaterials();
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


	void SDxRendererGM::OnKeyDown(double deltaTime)
	{
		auto dt = deltaTime;

		if (GetAsyncKeyState('W') & 0x8000)
			mCamera.Walk(30.0f * dt);

		if (GetAsyncKeyState('S') & 0x8000)
			mCamera.Walk(-30.0f * dt);

		if (GetAsyncKeyState('A') & 0x8000)
			mCamera.Strafe(-30.0f * dt);

		if (GetAsyncKeyState('D') & 0x8000)
			mCamera.Strafe(30.0f * dt);

		if (GetAsyncKeyState('F') & 0x8000)
			mCamera.SetPosition(0.0f, 0.0f, 0.0f);

		mCamera.UpdateViewMatrix();
	}

	void SDxRendererGM::AnimateMaterials()
	{

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

				SRenderMeshConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

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
			DisneyPBRMaterial* mat = e.second.get();
			if (mat->NumFramesDirty > 0)
			{
				DisneyMaterialConstants matConstants;
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
		XMMATRIX view = mCamera.GetView();
		XMMATRIX proj = mCamera.GetProj();

		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
		XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

		XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
		mMainPassCB.EyePosW = mCamera.GetPosition3f();
		mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
		mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
		mMainPassCB.NearZ = 1.0f;
		mMainPassCB.FarZ = 1000.0f;
		mMainPassCB.TotalTime = 1;
		mMainPassCB.DeltaTime = 1;
		mMainPassCB.AmbientLight = { .25f, .25f, .35f, 1.f };
		mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
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
		
		mSsaoPass->GetOffsetVectors(mSsaoCB.OffsetVectors);

		auto blurWeights = mSsaoPass->CalcGaussWeights(2.5f);
		mSsaoCB.BlurWeights[0] = XMFLOAT4(&blurWeights[0]);
		mSsaoCB.BlurWeights[1] = XMFLOAT4(&blurWeights[4]);
		mSsaoCB.BlurWeights[2] = XMFLOAT4(&blurWeights[8]);

		mSsaoCB.InvRenderTargetSize = XMFLOAT2(1.0f / (mClientWidth), 1.0f / (mClientHeight));

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
		std::vector<std::string> texNames =
		{
			"DiffTex",
			"RoughTex",
			"SpecTex",
			"NormalTex",
			"SkyCubeMap"
		};
		std::vector<std::wstring> texFilenames =
		{
			L"Textures/Urn_ALB.dds",
			L"Textures/Urn_RMA.dds",
			L"Textures/Urn_RMA.dds",
			L"Textures/Urn_NRM.dds",
			L"Textures/grasscube1024.dds"
		};
		for (int i = 0; i < (int)texNames.size(); ++i)
		{
			auto texMap = std::make_unique<SD3DTexture>();
			texMap->Name = texNames[i];
			texMap->Filename = texFilenames[i];
			ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
				mCommandList.Get(), texMap->Filename.c_str(),
				texMap->Resource, texMap->UploadHeap));

			mTextures[texMap->Name] = std::move(texMap);
		}

		mHDRTexture = d3dUtil::LoadHDRTexture(md3dDevice.Get(), mCommandList.Get(), "HDRTexture", L"Textures/flower_road_4k.hdr");
		mBRDF_LUT = d3dUtil::LoadHDRTexture(md3dDevice.Get(), mCommandList.Get(), "BRDFTexture", L"Textures/COMMON_LUT_PBR.hdr", 3);
		//mBRDF_LUT = d3dUtil::LoadHDRTexture(md3dDevice.Get(), mCommandList.Get(), "BRDF_LUT", L"Textures/COMMON_LUT_PBR.png");
	}

	// GBuffer Pass RootSig.
	void SDxRendererGM::BuildGBufferPassRootSignature()
	{
		// Albedo RMO Spec Normal
		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			GBufferResourceSrv,  // number of descriptors
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
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(md3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mRootSignatures["GBufferPass"].GetAddressOf())));
	}


	void SDxRendererGM::BuildDeferredPassRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE texTable, texTable1, texTable2, texTable3, texTable4, texTable5;
		texTable.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,  // number of descriptors
			0); // register t0
		texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,
			1);
		texTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,
			2);
		texTable3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,
			3);
		texTable4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			6,
			4);
		texTable5.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,  // number of descriptors
			10); // register t5

		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[9];

		// Create root CBV
		slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[1].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[2].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[3].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[4].InitAsConstantBufferView(0);
		slotRootParameter[5].InitAsConstantBufferView(1);
		slotRootParameter[6].InitAsConstantBufferView(2);
		slotRootParameter[7].InitAsDescriptorTable(1, &texTable4, D3D12_SHADER_VISIBILITY_PIXEL);
		slotRootParameter[8].InitAsDescriptorTable(1, &texTable5, D3D12_SHADER_VISIBILITY_PIXEL);

		auto staticSamplers = HikaD3DUtils::GetStaticSamplers();

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(9, slotRootParameter,
			(UINT)staticSamplers.size(), staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// Create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer.
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(md3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mRootSignatures["DeferredShading"].GetAddressOf())));
	}

	void SDxRendererGM::BuildDescriptorHeaps()
	{
		//
		// Create the SRV heap.
		//
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = GBufferResourceSrv;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mGBufferSrvDescriptorHeap)));


		srvHeapDesc.NumDescriptors = GBufferRTNum + GBufferSrvStartAt + LUTNum;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mDeferredSrvDescriptorHeap)));
	}

	void SDxRendererGM::BindPassResources()
	{
		auto DiffTex = mTextures["DiffTex"]->Resource;
		auto RoughTex = mTextures["RoughTex"]->Resource;
		auto SpecTex = mTextures["SpecTex"]->Resource;
		auto NormTex = mTextures["NormalTex"]->Resource;
		auto SkyTex = mTextures["SkyCubeMap"]->Resource;
		

		mSkyCubeResource.push_back(SkyTex.Get());

		mGBufferSrvResources.push_back(DiffTex.Get());
		mGBufferSrvResources.push_back(RoughTex.Get());
		mGBufferSrvResources.push_back(SpecTex.Get());
		mGBufferSrvResources.push_back(NormTex.Get());


		// Fill out the heap with actual descriptors:
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mGBufferSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		// LOD Clamp 0, ban nega val
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		// Iterate
		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		for (int i = 1; i < mGBufferSrvResources.size(); i++)
		{
			auto resource = mGBufferSrvResources[i];
			srvDesc.Format = resource->GetDesc().Format;
			srvDesc.Texture2D.MipLevels = resource->GetDesc().MipLevels;
			md3dDevice->CreateShaderResourceView(resource, &srvDesc, hDescriptor);
			hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		}
	}


	void SDxRendererGM::BuildGBufferPassDescriptorHeaps()
	{
		// Albedo RMO Spec Normal
		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			GBufferResourceSrv,  // number of descriptors
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
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(md3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(mRootSignatures["GBufferPass"].GetAddressOf())));
	}

	void SDxRendererGM::BuildDeferredShadingPassDescriptorHeaps()
	{
		//
		// Fill out the heap with actual descriptors.
		//
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mDeferredSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		
		auto GBufferAlbedo = GBufferRTs[0].get();
		auto GBufferNormal = GBufferRTs[1].get();
		auto GBufferWPos = GBufferRTs[2].get();
		auto GBufferRMO = GBufferRTs[3].get();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		hDescriptor.Offset(GBufferSrvStartAt, mCbvSrvDescriptorSize);
		srvDesc.Format = mBRDF_LUT->Resource->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		md3dDevice->CreateShaderResourceView(mBRDF_LUT->Resource.Get(), &srvDesc, hDescriptor);

		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		srvDesc.Format = GBufferAlbedo->mProperties.mRtvFormat;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		md3dDevice->CreateShaderResourceView(GBufferAlbedo->mResource.Get(), &srvDesc, hDescriptor);
		
		// next descriptor: GBufferNormal
		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		mSsaoSrvResources.push_back(GBufferRTs[1]->mResource.Get());
		mSsaoSrvResources.push_back(mDepthStencilBuffer.Get());
		srvDesc.Format = GBufferNormal->mProperties.mRtvFormat;
		md3dDevice->CreateShaderResourceView(GBufferNormal->mResource.Get(), &srvDesc, hDescriptor);

		// next descriptor: GBufferWPos
		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		srvDesc.Format = GBufferWPos->mProperties.mRtvFormat;
		md3dDevice->CreateShaderResourceView(GBufferWPos->mResource.Get(), &srvDesc, hDescriptor);

		// next descriptor: GBufferRMO
		hDescriptor.Offset(1, mCbvSrvDescriptorSize);
		srvDesc.Format = GBufferRMO->mProperties.mRtvFormat;
		md3dDevice->CreateShaderResourceView(GBufferRMO->mResource.Get(), &srvDesc, hDescriptor);
	}


	void SDxRendererGM::BuildShaderAndInputLayout()
	{
		mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\StandardVS.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\DisneyPBRShader.hlsl", nullptr, "PS", "ps_5_1");
				
		mInputLayouts[SPasses::E_Deferred] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}


	void SDxRendererGM::BuildGBufferPassShaderAndInputLayout()
	{
		mShaders["GBufferPS"] = d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\GBuffer.hlsl", nullptr, "PS", "ps_5_1");

		mInputLayouts[SPasses::E_GBuffer] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}


	void SDxRendererGM::BuildGBufferDebugShaderAndInputLayout()
	{
		mShaders["GBufferDebugVS"]
			= d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\GBufferDebug.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["GBufferDebugPS"]
			= d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\GBufferDebug.hlsl", nullptr, "PS_GBufferDebugging", "ps_5_1");
		mInputLayouts[SPasses::E_GBufferDebug] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TYPE", 0, DXGI_FORMAT_R16_UINT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void SDxRendererGM::BuildDeferredShadingPassShaderAndInputLayout()
	{
		mShaders["ScreenQuadVS"]
			= d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\ScreenQuadVS.hlsl", nullptr, "VS", "vs_5_1");
		mShaders["ScreenQuadVS_Portable"]
			= d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\ScreenQuadVS.hlsl", nullptr, "VS_Portable", "vs_5_1");
		mShaders["DeferredShading"]
			= d3dUtil::CompileShader(L"Shaders\\PBR\\Pipeline\\DisneyPBRShaderDeferred.hlsl", nullptr, "PS", "ps_5_1");

		mInputLayouts[SPasses::E_Deferred] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
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

		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = "ScreenQuad";

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(ScreenQuadVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		// Create Vertex Buffer Blob
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
		
#if defined(Sakura_SSAO_DEBUG) 
		{
		quadSubmesh.IndexCount = (UINT)quad.Indices32.size();
		quadSubmesh.StartIndexLocation = 0;
		quadSubmesh.BaseVertexLocation = 0;

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(ScreenQuadDebugVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		for (int j = 0; j < 5; j++)
		{
			auto geo = std::make_unique<MeshGeometry>();
			mGeometries["DebugScreenQuad" + j] = std::move(geo);

			std::vector<ScreenQuadDebugVertex> vertices(quadSubmesh.IndexCount);
			for (int i = 0; i < quad.Vertices.size(); ++i)
			{
				vertices[i].Pos = quad.Vertices[i].Position;
				vertices[i].TexC = quad.Vertices[i].TexC;
				vertices[i].Type = j;
			}
			std::vector<std::uint16_t> indices;
			indices.insert(indices.end(), std::begin(quad.GetIndices16()), std::end(quad.GetIndices16()));

			// Create Vertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeometries["DebugScreenQuad" + j]->VertexBufferCPU));
			CopyMemory(mGeometries["DebugScreenQuad" + j]->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeometries["DebugScreenQuad" + j]->IndexBufferCPU));
			CopyMemory(mGeometries["DebugScreenQuad" + j]->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			mGeometries["DebugScreenQuad" + j]->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, mGeometries["DebugScreenQuad" + j]->VertexBufferUploader);
			mGeometries["DebugScreenQuad" + j]->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, mGeometries["DebugScreenQuad" + j]->IndexBufferUploader);
			mGeometries["DebugScreenQuad" + j]->VertexByteStride = sizeof(ScreenQuadDebugVertex);
			mGeometries["DebugScreenQuad" + j]->VertexBufferByteSize = vbByteSize;
			mGeometries["DebugScreenQuad" + j]->IndexFormat = DXGI_FORMAT_R16_UINT;
			mGeometries["DebugScreenQuad" + j]->IndexBufferByteSize = ibByteSize;
			mGeometries["DebugScreenQuad" + j]->DrawArgs["DebugScreenQuad" + j] = quadSubmesh;
		}
		}
#endif
		
#if defined(Sakura_IBL)
		{
			auto sphere = geoGen.CreateSphere(0.5f, 20, 20);
			SubmeshGeometry quadSubmesh;
			quadSubmesh.IndexCount = (UINT)sphere.Indices32.size();
			quadSubmesh.StartIndexLocation = 0;
			quadSubmesh.BaseVertexLocation = 0;

			std::vector<Vertex> vertices(quadSubmesh.IndexCount);
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

			auto geo = std::make_unique<MeshGeometry>();
			geo->Name = "SkySphere";

			const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
			const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

			// Create Vertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
			CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
			geo->VertexByteStride = sizeof(Vertex);
			geo->VertexBufferByteSize = vbByteSize;
			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
			geo->IndexBufferByteSize = ibByteSize;

			geo->DrawArgs[geo->Name] = quadSubmesh;
			mGeometries[geo->Name] = std::move(geo);
		}
#endif
#if defined(Sakura_IBL_HDR_INPUT)
		{
			auto sphere = geoGen.CreateBox(1.f, 1.f, 1.f, 1);
			SubmeshGeometry quadSubmesh;
			quadSubmesh.IndexCount = (UINT)sphere.Indices32.size();
			quadSubmesh.StartIndexLocation = 0;
			quadSubmesh.BaseVertexLocation = 0;

			std::vector<Vertex> vertices(quadSubmesh.IndexCount);
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

			auto geo = std::make_unique<MeshGeometry>();
			geo->Name = "HDRCube";

			const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
			const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

			// Create Vertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
			CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
			geo->VertexByteStride = sizeof(Vertex);
			geo->VertexBufferByteSize = vbByteSize;
			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
			geo->IndexBufferByteSize = ibByteSize;

			geo->DrawArgs[geo->Name] = quadSubmesh;
			mGeometries[geo->Name] = std::move(geo);
		}
#endif
		
	}

	float TestScaling[3] = { 1.f, 1.f, 1.f };
	void SDxRendererGM::BuildGeometry()
	{
		// Link fbx
		mGeometries["skullGeo"] = std::move(MeshImporter::ImportMesh(md3dDevice.Get(), mCommandList.Get(), "Models/Urn.fbx", ESupportFileForm::ASSIMP_SUPPORTFILE));
		
		// PLY
		//mGeometries["skullGeo"] = std::move(MeshImporter::ImportMesh(md3dDevice.Get(), mCommandList.Get(), "Models/dragon_vrip.ply", ESupportFileForm::PLY));
		
		// text
		//mGeometries["skullGeo"] = std::move(MeshImporter::ImportMesh(md3dDevice.Get(), mCommandList.Get(), "Models/skull.txt"));
	}

	void SDxRendererGM::BuildPSOs()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

		// PSO for opaque objects.
		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		opaquePsoDesc.InputLayout = { mInputLayouts[SPasses::E_GBuffer].data(), (UINT)mInputLayouts[SPasses::E_GBuffer].size() };
		opaquePsoDesc.pRootSignature = mRootSignatures["GBufferPass"].Get();
		opaquePsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
			mShaders["standardVS"]->GetBufferSize()
		};
		opaquePsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
			mShaders["opaquePS"]->GetBufferSize()
		};
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		//opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 1;
		opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
		opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		opaquePsoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["ForwardShading"])));


#if defined(Sakura_Defferred)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gbufferPsoDesc = opaquePsoDesc;
		gbufferPsoDesc.InputLayout = { mInputLayouts[SPasses::E_GBuffer].data(), (UINT)mInputLayouts[SPasses::E_GBuffer].size() };
		gbufferPsoDesc.pRootSignature = mRootSignatures["GBufferPass"].Get();
		gbufferPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
			mShaders["standardVS"]->GetBufferSize()
		};
		gbufferPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["GBufferPS"]->GetBufferPointer()),
			mShaders["GBufferPS"]->GetBufferSize()
		};
		gbufferPsoDesc.NumRenderTargets = GBufferRTNum;
		for (int i = 0; i < GBufferRTNum; i++)
		{
			gbufferPsoDesc.RTVFormats[i] = GBufferRTs[i]->mProperties.mRtvFormat;
		}
		gbufferPsoDesc.SampleDesc.Count = 1;
		gbufferPsoDesc.SampleDesc.Quality = 0;
		gbufferPsoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&gbufferPsoDesc, IID_PPV_ARGS(&mPSOs["GBufferPass"])));

		// Deferred Shading Pass
		D3D12_GRAPHICS_PIPELINE_STATE_DESC deferredPsoDesc = opaquePsoDesc;
		deferredPsoDesc.DepthStencilState.DepthEnable = false;
		deferredPsoDesc.pRootSignature = mRootSignatures["DeferredShading"].Get();
		deferredPsoDesc.InputLayout = { mInputLayouts[SPasses::E_Deferred].data(), (UINT)mInputLayouts[SPasses::E_Deferred].size() };
		gbufferPsoDesc.NumRenderTargets = 1;
		deferredPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		deferredPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		deferredPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["ScreenQuadVS"]->GetBufferPointer()),
			mShaders["ScreenQuadVS"]->GetBufferSize()
		};
		deferredPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["DeferredShading"]->GetBufferPointer()),
			mShaders["DeferredShading"]->GetBufferSize()
		};
		deferredPsoDesc.RTVFormats[0] = mBackBufferFormat;
		deferredPsoDesc.SampleDesc.Count = 1;
		deferredPsoDesc.SampleDesc.Quality = 0;
		deferredPsoDesc.DSVFormat = mDepthStencilFormat;
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&deferredPsoDesc, IID_PPV_ARGS(&mPSOs["DeferredShading"])));
#endif

#if defined(Sakura_SSAO_DEBUG) 
		D3D12_GRAPHICS_PIPELINE_STATE_DESC debugPsoDesc = deferredPsoDesc;
		debugPsoDesc.InputLayout = { mInputLayouts[SPasses::E_GBufferDebug].data(), (UINT)mInputLayouts[SPasses::E_GBufferDebug].size() };
		debugPsoDesc.pRootSignature = mRootSignatures["DeferredShading"].Get();
		debugPsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(mShaders["GBufferDebugVS"]->GetBufferPointer()),
			mShaders["GBufferDebugVS"]->GetBufferSize()
		};
		debugPsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(mShaders["GBufferDebugPS"]->GetBufferPointer()),
			mShaders["GBufferDebugPS"]->GetBufferSize()
		};
		ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&debugPsoDesc, IID_PPV_ARGS(&mPSOs["GBufferDebug"])));
#endif	
	}

	void SDxRendererGM::BuildFrameResources()
	{
		for (int i = 0; i < gNumFrameResources; i++)
		{
			mFrameResources.push_back(std::make_unique<SFrameResource>(md3dDevice.Get(),
				1 + 6 + SkyCubeConvFilterNum * 6, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
		}
	}

	void SDxRendererGM::BuildMaterials()
	{
		int MatCBInd = 0;
		auto bricks0 = std::make_unique<DisneyPBRMaterial>();
		bricks0->Name = "bricks0";
		bricks0->MatCBIndex = MatCBInd++;
		bricks0->MatConstants.DiffuseSrvHeapIndex = 0;
		bricks0->MatConstants.RMOSrvHeapIndex = 1;
		bricks0->MatConstants.SpecularSrvHeapIndex = 2;
		bricks0->MatConstants.NormalSrvHeapIndex = 3;
		bricks0->MatConstants.BaseColor = XMFLOAT3(Colors::White);
		bricks0->MatConstants.Roughness = 1.f;
		GBufferResourceSrv += 4;
		GBufferMaterials += 1;
		mMaterials["bricks0"] = std::move(bricks0);

#if defined(Sakura_Debug_PureColor)
		for (int j = 0; j < 11; j++)
		{
			for (int i = 0; i < 11; i++)
			{
				auto testM = std::make_unique<DisneyPBRMaterial>();
				testM->Name = "test" + std::to_string(i) + std::to_string(j);
				testM->MatCBIndex = MatCBInd++;
				testM->MatConstants.DiffuseSrvHeapIndex = -1;
				testM->MatConstants.RMOSrvHeapIndex = -1;
				testM->MatConstants.SpecularSrvHeapIndex = -1;
				testM->MatConstants.NormalSrvHeapIndex = -1;
				testM->MatConstants.BaseColor = XMFLOAT3(Colors::White);
				testM->MatConstants.Roughness = .1f * (float)i;
				testM->MatConstants.Metallic = .1f * (float)j;
				mMaterials[testM->Name] = std::move(testM);
			}
		}
#endif
	}
	
	void SDxRendererGM::BuildRenderItems()
	{
		int CBIndex = 0;
#if defined(Sakura_Full_Effects)
		auto opaqueRitem = std::make_unique<SRenderItem>();
		XMStoreFloat4x4(&opaqueRitem->World, XMMatrixScaling(TestScaling[0], TestScaling[1], TestScaling[2]) * 
		XMMatrixTranslation(0.f, -6.f, 0.f) * XMMatrixRotationY(155));
		opaqueRitem->TexTransform = MathHelper::Identity4x4();
		opaqueRitem->ObjCBIndex = CBIndex++;
		opaqueRitem->Mat = mMaterials["bricks0"].get();
		opaqueRitem->Geo = mGeometries["skullGeo"].get();
		opaqueRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		opaqueRitem->IndexCount = opaqueRitem->Geo->DrawArgs["mesh"].IndexCount;
		opaqueRitem->StartIndexLocation = opaqueRitem->Geo->DrawArgs["mesh"].StartIndexLocation;
		opaqueRitem->BaseVertexLocation = opaqueRitem->Geo->DrawArgs["mesh"].BaseVertexLocation;
		mRenderLayers[SRenderLayers::E_Opaque].push_back(opaqueRitem.get());
		mAllRitems.push_back(std::move(opaqueRitem));
#elif defined(Sakura_Debug_PureColor)
		for (int j = 0; j < 11; j++)
		{
			for (int i = 0; i < 11; i++)
			{
				auto sphere = std::make_unique<SRenderItem>();
				XMStoreFloat4x4(&sphere->World, XMMatrixScaling(1.8f, 1.8f, 1.8f) *
					XMMatrixTranslation(2.5f * i, 0.f, 2.5f * j));
				sphere->TexTransform = MathHelper::Identity4x4();
				sphere->ObjCBIndex = CBIndex++;
				std::string name = "test" + std::to_string(i) + std::to_string(j);
				sphere->Mat = mMaterials[name].get();
				sphere->Mat->MatConstants.Roughness = 0.1 * i;
				sphere->Geo = mGeometries["SkySphere"].get();
				sphere->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				sphere->IndexCount = sphere->Geo->DrawArgs["SkySphere"].IndexCount;
				sphere->StartIndexLocation = sphere->Geo->DrawArgs["SkySphere"].StartIndexLocation;
				sphere->BaseVertexLocation = sphere->Geo->DrawArgs["SkySphere"].BaseVertexLocation;
				mRenderLayers[SRenderLayers::E_Opaque].push_back(sphere.get());
				mAllRitems.push_back(std::move(sphere));
			}
		}
#endif

		auto screenQuad = std::make_unique<SRenderItem>();
		screenQuad->World = MathHelper::Identity4x4();
		screenQuad->TexTransform = MathHelper::Identity4x4();
		screenQuad->ObjCBIndex = CBIndex++;
		screenQuad->Mat = mMaterials["bricks0"].get();
		screenQuad->Geo = mGeometries["ScreenQuad"].get();
		screenQuad->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		screenQuad->IndexCount = screenQuad->Geo->DrawArgs["ScreenQuad"].IndexCount;
		screenQuad->StartIndexLocation = screenQuad->Geo->DrawArgs["ScreenQuad"].StartIndexLocation;
		screenQuad->BaseVertexLocation = screenQuad->Geo->DrawArgs["ScreenQuad"].BaseVertexLocation;

		mRenderLayers[SRenderLayers::E_ScreenQuad].push_back(screenQuad.get());
		mAllRitems.push_back(std::move(screenQuad));

#if defined(Sakura_IBL)
		auto skyRitem = std::make_unique<SRenderItem>();
		XMStoreFloat4x4(&skyRitem->World, XMMatrixScaling(5000.f, 5000.f, 5000.f));
		skyRitem->TexTransform = MathHelper::Identity4x4();
		skyRitem->ObjCBIndex = CBIndex++;
		skyRitem->Mat = mMaterials["bricks0"].get();
		skyRitem->Geo = mGeometries["SkySphere"].get();
		skyRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		skyRitem->IndexCount = skyRitem->Geo->DrawArgs["SkySphere"].IndexCount;
		skyRitem->StartIndexLocation = skyRitem->Geo->DrawArgs["SkySphere"].StartIndexLocation;
		skyRitem->BaseVertexLocation = skyRitem->Geo->DrawArgs["SkySphere"].BaseVertexLocation;

		mRenderLayers[SRenderLayers::E_SKY].push_back(skyRitem.get());
		mAllRitems.push_back(std::move(skyRitem));
#endif

#if defined(Sakura_IBL_HDR_INPUT)
		auto boxRitem = std::make_unique<SRenderItem>();
		XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(1000.f, 1000.f, 1000.f));
		boxRitem->TexTransform = MathHelper::Identity4x4();
		boxRitem->ObjCBIndex = CBIndex++;
		boxRitem->Mat = mMaterials["bricks0"].get();
		boxRitem->Geo = mGeometries["HDRCube"].get();
		boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		boxRitem->IndexCount = boxRitem->Geo->DrawArgs["HDRCube"].IndexCount;
		boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["HDRCube"].StartIndexLocation;
		boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["HDRCube"].BaseVertexLocation;

		mRenderLayers[SRenderLayers::E_Cube].push_back(boxRitem.get());
		mAllRitems.push_back(std::move(boxRitem));
#endif


#if defined(Sakura_SSAO_DEBUG) 
		for (int i = 0; i < 6; i++)
		{
			std::string Name = "DebugScreenQuad" + i;
			screenQuad = std::make_unique<SRenderItem>();
			screenQuad->World = MathHelper::Identity4x4();
			screenQuad->TexTransform = MathHelper::Identity4x4();
			screenQuad->ObjCBIndex = CBIndex++;
			screenQuad->Mat = mMaterials["bricks0"].get();
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

	void SDxRendererGM::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<SRenderItem*>& ritems)
	{
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(SRenderMeshConstants));
		UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

		auto objectCB = mCurrFrameResource->ObjectCB->Resource();
		auto matCB = mCurrFrameResource->MaterialCB->Resource();

		// For each render item...
		for (size_t i = 0; i < ritems.size(); ++i)
		{
			auto ri = ritems[i];

			cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
			D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

			cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
			cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}

	}

	void SDxRendererGM::CreateRtvAndDsvDescriptorHeaps()
	{
		//Create render target view descriptor for swap chain buffers
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		// 6 * 5 : Cubemap with 5 mips
		// 6  : CubeMap irradiance 
		// 30 : 5 * 6 roughness prefiltered cube maps.
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount + GBufferRTNum + 
			6 * SkyCubeMips + 6 * SkyCubeConvFilterNum + 1;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
			&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

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