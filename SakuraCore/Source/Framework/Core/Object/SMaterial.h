#pragma once
#include "Interface\ISObject.h"
#include "Framework\GraphicTypes\D3D12\DX12Definations.h"

namespace SGraphics
{
	// SObject wrap of Opaque Material.
	// With Guid.
	class SMaterial : SImplements SakuraCore::ISSlientObject
	{
	public:
		SMaterial()
		{
			ISSlientObject::ISSlientObject();
		}
		~SMaterial() {}
		union 
		{
			OpaqueMaterial data;
		};
		SIndex indexInScene = 0;
	};
}