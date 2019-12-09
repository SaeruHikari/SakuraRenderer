#pragma once
#include "Interface/IRuntimeModule.h"

namespace SGraphics
{
	class SakuraGraphicsResourceManagerBase : SImplements SakuraCore::IRuntimeModule
	{
	protected:
		// Hide constructors to prevent unexpected instance for virtual class.
		SakuraGraphicsResourceManagerBase()
			:IRuntimeModule()
		{

		}
		SakuraGraphicsResourceManagerBase(const SakuraGraphicsResourceManagerBase& rhs) = delete;
		SakuraGraphicsResourceManagerBase& operator=(const SakuraGraphicsResourceManagerBase& rhs) = delete;
	public:
		~SakuraGraphicsResourceManagerBase() {};
		// Create GPU resource for material.
		virtual void CreateMaterial() = 0;
		// Release GPU Resource of Material.
		virtual void ReleaseMaterial() = 0;
	};
}