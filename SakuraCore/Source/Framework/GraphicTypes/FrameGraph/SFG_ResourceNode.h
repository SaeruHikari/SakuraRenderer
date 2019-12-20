#pragma once
#include "Interface/ISObject.h"
#include "Framework/GraphicTypes/GraphicsCommon/GraphicsConfigs.h"

namespace SGraphics
{
	class ISRenderResource;
	class SakuraFrameGraph;
	class SakuraGraphicsResourceManagerBase;
}

namespace SGraphics
{
	class SFG_ResourceNode : SImplements SakuraCore::ISSilentObject
	{
		friend class SGraphics::SakuraFrameGraph;
	public:
		SFG_ResourceNode(SakuraFrameGraph* frameGraph, ISRenderResource* resource)
			:ISSilentObject()
		{
			pFrameGraph = frameGraph;
			mResource = resource;
		}
		__forceinline ISRenderResource* GetResource()
		{
			return mResource;
		}
		std::unique_ptr<SFG_ResourceNode> Duplicate()
		{
			auto copy = std::make_unique<SFG_ResourceNode>(pFrameGraph, mResource);
			copy->mName = mName;
			return copy;
		}
	private:
		size_t refs = 0;
		std::string mName = "NULL";
		SFG_PassNode* writer = nullptr;
		SGraphics::ISRenderResource* mResource = nullptr;
		SakuraFrameGraph* pFrameGraph = nullptr;
	};
}