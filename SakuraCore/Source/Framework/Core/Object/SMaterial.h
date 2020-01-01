#pragma once
#include "Interface\ISObject.h"
#include "Framework\GraphicTypes\D3D12\DX12Definations.h"

using namespace SakuraCore;

namespace SGraphics
{
	// SObject wrap of Opaque Material.
	// With Guid.
	class SMaterial : SImplements SakuraCore::ISSilentObject
	{
	public:
		SMaterial()
		{
			ISSilentObject::ISSilentObject();
		}
		~SMaterial() {}
		OpaqueMaterial data;
		SIndex indexInScene = 0;
	};
}