#pragma once
#include "../../pch.h"
#include "DX12DeviceInitSystem.hpp"

using namespace SECS;
using namespace Microsoft::WRL;

namespace SakuraGraphics {
	class DX12DeviceSystemGroup : public SSystemGroup
	{
	public:
		DX12DeviceSystemGroup()
		{
			SSystemGroup::SSystemGroup();

			ADD_SYSTEM_SUBGROUP(DX12DeviceInitSystem, "InitSystem");
		}

	};
	REGISTRY_SYSTEM_TO_ROOT_GROUP(DX12DeviceSystemGroup, DX12Device)
}