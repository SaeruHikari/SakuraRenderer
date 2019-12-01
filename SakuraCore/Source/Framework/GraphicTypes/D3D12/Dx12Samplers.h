#pragma once
#include <array>
#include "Common/Microsoft/d3dx12.h"

namespace HikaD3DUtils
{
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();	
}