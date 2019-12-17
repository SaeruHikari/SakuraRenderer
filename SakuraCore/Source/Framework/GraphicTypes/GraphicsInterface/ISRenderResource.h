#pragma once
#include "..\GraphicsCommon\GraphicsConfigs.h"
#include "SakuraCore_ECS\Source\Graphics\Utils\d3dx12.h"

namespace SGraphics
{
	struct SResourceHandle
	{
	public:
		SResourceCPUHandle hCpu;
		SResourceGPUHandle hGpu;
	};

	struct ISRenderResource
	{
		virtual SRHIResource* GetGPUResource() = 0;
		virtual SResourceHandle* GetResourceHandle() = 0;
	};

}