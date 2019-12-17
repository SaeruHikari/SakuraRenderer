#pragma once
#include "ISRenderResource.h"
#include <wrl\client.h>

namespace SGraphics
{
	struct ISTexture : public ISRenderResource
	{
		ISTexture()
		{
		}
		virtual SRHIResource* GetGPUResource() override
		{
			return Resource.Get();
		}
		virtual SResourceHandle* GetResourceHandle() = 0;
		Microsoft::WRL::ComPtr<SRHIResource> Resource;
	};
}