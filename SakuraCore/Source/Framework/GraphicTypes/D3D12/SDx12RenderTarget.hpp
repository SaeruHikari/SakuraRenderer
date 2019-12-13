#pragma once
#include "D3DCommon.h"
#include <d3d12.h>
#include "..\GraphicsInterface\ISRenderTarget.h"
#include "SDescriptorHeap.hpp"

namespace SGraphics
{
	class ISDx12RenderTarget : public ISRenderTarget
	{
	protected:
	public:
		~ISDx12RenderTarget() {};
		virtual void OnResize(ID3D12Device* device, UINT ClientWidth, UINT ClientHeight) = 0;
		virtual ID3D12Resource* Resource() = 0;
	protected:
		bool Initialized = false;
	};

	class SDx12RenderTarget2D : public ISDx12RenderTarget
	{
	public:
		SDx12RenderTarget2D(UINT ClientWidth, UINT ClientHeight, ISRenderTargetProperties prop = {}, bool ScaledByViewport = true)
		{
			mProperties = prop;
			
			mProperties.mWidth = ClientWidth;
			mProperties.mHeight = ClientHeight;

			bScaledByViewport = ScaledByViewport;

			if (bScaledByViewport)
				SetViewportAndScissorRect((UINT)(ClientWidth * mProperties.mWidthPercentage), (UINT)(ClientHeight * mProperties.mHeightPercentage));
			else
				SetViewportAndScissorRect(ClientWidth, ClientHeight);
		}
		virtual ID3D12Resource* Resource()
		{
			return mResource.Get();
		}

		~SDx12RenderTarget2D() {};
		
		void BuildDescriptors(D3D12_RESOURCE_DESC desc, ID3D12Device* md3dDevice,
			SDescriptorHeap* rtv,
			SDescriptorHeap* srv)
		{
			// Init RT
			CD3DX12_CLEAR_VALUE optClear(desc.Format, mProperties.mClearColor);
#if defined(RESERVE_Z)
			optClear.DepthStencil.Depth = 0;
#else
			optClear.DepthStencil.Depth = 1;
#endif
			ThrowIfFailed(md3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&mResource)));

			// Create Cpu Rtv
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			rtvDesc.Format = mProperties.mRtvFormat;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;
			if (!Initialized)
			{
				mSRtv = rtv->GetAvailableHandle();
				mSSrv = srv->GetAvailableHandle();
				Initialized = true;
			}
			md3dDevice->CreateRenderTargetView(mResource.Get(), &rtvDesc, mSRtv.hCpu);
			CreateShaderResourceView(md3dDevice, mProperties.mRtvFormat);
		}

		void BuildDescriptors(ID3D12Device* md3dDevice,
			SDescriptorHeap* rtv,
			SDescriptorHeap* srv)
		{
			D3D12_RESOURCE_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Alignment = 0;
			texDesc.Width = mProperties.mWidth;
			texDesc.Height = mProperties.mHeight;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.Format = mProperties.mRtvFormat;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			BuildDescriptors(texDesc, md3dDevice, rtv, srv);
		}

		virtual void OnResize(ID3D12Device* md3dDevice, UINT ClientWidth, UINT ClientHeight)
		{
			if (bScaledByViewport)
			{
				mProperties.mWidth = ClientWidth;
				mProperties.mHeight = ClientHeight;
				SetViewportAndScissorRect((UINT)(ClientWidth * mProperties.mWidthPercentage), (UINT)(ClientHeight * mProperties.mHeightPercentage));
				BuildDescriptors(md3dDevice);
			}
		}
	public:
		void ClearRenderTarget(ID3D12GraphicsCommandList* cmdList)
		{
			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);
			cmdList->ClearRenderTargetView(mSRtv.hCpu, mProperties.mClearColor, 0, nullptr);
		}
	private:
		void BuildDescriptors(ID3D12Device* md3dDevice)
		{
			D3D12_RESOURCE_DESC texDesc;//
			ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Alignment = 0;
			texDesc.Width = mProperties.mWidth;
			texDesc.Height = mProperties.mHeight;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.Format = mProperties.mRtvFormat;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			// Init RT
			CD3DX12_CLEAR_VALUE optClear(texDesc.Format, mProperties.mClearColor);
			ThrowIfFailed(md3dDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&mResource)));

			// Create Cpu Rtv
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			rtvDesc.Format = mProperties.mRtvFormat;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;
			md3dDevice->CreateRenderTargetView(mResource.Get(), &rtvDesc, mSRtv.hCpu);
			CreateShaderResourceView(md3dDevice, mProperties.mRtvFormat);
		}

		void CreateShaderResourceView(ID3D12Device* md3dDevice, DXGI_FORMAT format)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = format;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			md3dDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, mSSrv.hCpu);
		}
		void SetViewportAndScissorRect(UINT Width, UINT Height)
		{
			mViewport.TopLeftX = 0.0f;
			mViewport.TopLeftY = 0.0f;
			mViewport.Width = (FLOAT)Width;
			mViewport.Height = (FLOAT)Height;
			mViewport.MinDepth = 0.0f;
			mViewport.MaxDepth = 1.0f;

			mScissorRect = { 0, 0, (int)(mViewport.Width), (int)(mViewport.Height) };
		}
	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> GetResource()
		{
			return mResource;
		}
		Microsoft::WRL::ComPtr<ID3D12Resource> mResource;

		DescriptorHandleCouple mSRtv;
		DescriptorHandleCouple mSSrv;

		D3D12_VIEWPORT mViewport;
		D3D12_RECT mScissorRect;

		bool bScaledByViewport = true;
	};

	class SDx12RenderTargetCube : public ISDx12RenderTarget
	{
	public:
		SDx12RenderTargetCube(
			UINT width, UINT height,
			DXGI_FORMAT format)
		{
			mProperties.mWidth = width;
			mProperties.mHeight = height;
			mFormat = format;

			mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
			mScissorRect = { 0, 0, (int)width, (int)height };
		}

		SDx12RenderTargetCube(const SDx12RenderTargetCube& rhs) = delete;
		SDx12RenderTargetCube& operator=(const SDx12RenderTargetCube& rhs) = delete;
		~SDx12RenderTargetCube() = default;

		virtual ID3D12Resource* Resource()
		{
			return mCubeMap.Get();
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE Srv()
		{
			return mSSrv.hGpu;
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(int faceIndex)
		{
			return mSRtv[faceIndex].hCpu;
		}

		D3D12_VIEWPORT Viewport()const { return mViewport; }
		D3D12_RECT ScissorRect()const { return mScissorRect; }

		void BuildDescriptors(
			ID3D12Device* device,
			SDescriptorHeap* rtv,
			SDescriptorHeap* srv)
		{
			if (!Initialized)
			{
				// Save references to the descriptors. 
				mSSrv = srv->GetAvailableHandle();
				for (size_t i = 0; i < 6; i++)
					mSRtv[i] = rtv->GetAvailableHandle();
				Initialized = true;
			}
			//  Create the descriptors
			BuildDescriptors(device);
		}

		void OnResize(ID3D12Device* device, UINT newWidth, UINT newHeight)
		{
			if ((mProperties.mWidth != newWidth) || (mProperties.mHeight != newHeight))
			{
				mProperties.mWidth = newWidth;
				mProperties.mHeight = newHeight;
			}
		}
	public:
		void ClearRenderTarget(ID3D12GraphicsCommandList* cmdList)
		{
			cmdList->RSSetViewports(1, &mViewport);
			cmdList->RSSetScissorRects(1, &mScissorRect);
			
			for(size_t i = 0; i < 6; i++)
				cmdList->ClearRenderTargetView(mSRtv[i].hCpu, mClearColor, 0, nullptr);
		}

	private:
		void BuildDescriptors(ID3D12Device* device)
		{
			BuildResource(device);
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = mFormat;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = 1;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

			// Create SRV to the entire cubemap resource.
			device->CreateShaderResourceView(mCubeMap.Get(), &srvDesc, mSSrv.hCpu);

			// Create RTV to each cube face.
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Format = mFormat;
			
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
			// Only view one element of the array.
			rtvDesc.Texture2DArray.ArraySize = 1;
			for (int i = 0; i < 6; ++i)
			{
				// Render target to ith element.
				rtvDesc.Texture2DArray.FirstArraySlice = i;
				// Create RTV to ith cubemap face.
				device->CreateRenderTargetView(mCubeMap.Get(), &rtvDesc, mSRtv[i].hCpu);
			}
		}
		void BuildResource(ID3D12Device* device)
		{
			D3D12_RESOURCE_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Alignment = 0;
			texDesc.Width = mProperties.mWidth;
			texDesc.Height = mProperties.mHeight;
			texDesc.DepthOrArraySize = 6;
			texDesc.MipLevels = 1;
			texDesc.Format = mFormat;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			
			CD3DX12_CLEAR_VALUE optClear(mFormat, mClearColor);
			ThrowIfFailed(device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&texDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear,
				IID_PPV_ARGS(&mCubeMap)));
		}

	private:
		FLOAT mClearColor[4] = { 1,0,0,0 };
		D3D12_VIEWPORT mViewport;
		D3D12_RECT mScissorRect;
		DXGI_FORMAT mFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DescriptorHandleCouple mSRtv[6];
		DescriptorHandleCouple mSSrv;
		Microsoft::WRL::ComPtr<ID3D12Resource> mCubeMap = nullptr;
	};
}