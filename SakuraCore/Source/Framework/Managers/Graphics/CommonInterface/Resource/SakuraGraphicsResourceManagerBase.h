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
		// Initialize function.
		virtual bool Initialize() = 0;
		// Finalize function.
		virtual void Finalize() = 0;
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime) = 0;
		//
		
	};
}