#pragma once
#include "../../CommonInterface/Resource/SakuraGraphicsResourceManagerBase.h"

namespace SGraphics
{
	class SDxResourceManager : public SakuraGraphicsResourceManagerBase 
	{
	public:
		SDxResourceManager();
		SDxResourceManager(const SDxResourceManager& rhs) = delete;
		SDxResourceManager& operator=(const SDxResourceManager&) = delete;
		~SDxResourceManager();
	public:
		// Create GPU resource for material.
		virtual void CreateMaterial() override;
		// Release GPU Resource of Material.
		virtual void ReleaseMaterial() override;
	private:

	};
}