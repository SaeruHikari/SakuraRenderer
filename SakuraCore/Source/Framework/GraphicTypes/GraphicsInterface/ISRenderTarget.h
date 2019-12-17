#pragma once
#include <minwindef.h>
#include <dxgiformat.h>
#include "ISRenderResource.h"
#include <wrl\client.h>

namespace SGraphics
{
	enum class ERenderTargetTypes : int
	{
		E_RT2D = 0,
		E_RT3D = 1
	};
	struct ISRenderTargetProperties
	{
		ISRenderTargetProperties() = default;
		ISRenderTargetProperties(float r, float g, float b, float a)
		{
			mClearColor[0] = r;
			mClearColor[1] = g;
			mClearColor[2] = b;
			mClearColor[3] = a;
		}
	public:
		//bool bScaleWithViewport = true;
		FLOAT mClearColor[4] = { 0,0,0,0 };
		ERenderTargetTypes rtType = ERenderTargetTypes::E_RT2D;
		DXGI_FORMAT mRtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		// RtvWidth = viewportWidth * mWidthPercentage
		UINT mWidth;
		UINT mHeight;
		float mWidthPercentage = 1;
		float mHeightPercentage = 1;
		bool bScaleWithViewport = false;
		~ISRenderTargetProperties() {};
	};

	struct ISRenderTarget : public ISRenderResource
	{
		ISRenderTargetProperties mProperties;
		~ISRenderTarget() 
		{
		};
		virtual SRHIResource* GetGPUResource() override
		{
			return mResource.Get();
		}
		virtual SResourceHandle* GetResourceHandle() 
		{
			return nullptr;
		}
		virtual SResourceHandle* GetRenderTargetHandle(size_t num = 0)
		{
			return nullptr;
		}
		Microsoft::WRL::ComPtr<SRHIResource> mResource;
	};
}