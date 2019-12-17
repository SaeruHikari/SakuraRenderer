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
	public:
		SFG_ResourceNode(SakuraFrameGraph* frameGraph, ISRenderResource* resource)
			:ISSilentObject()
		{
			pFrameGraph = frameGraph;
			mResource = resource;
		}
		inline void AddRef(SFG_PassNode* _writer)
		{
			writers.push_back(_writer);
			refs++;
		}
		__forceinline ISRenderResource* GetResource()
		{
			return mResource;
		}
	private:
		size_t refs = 0;
		std::string mName = "NULL";
		std::vector<SFG_PassNode*> writers;
		bool bValid = true;
		SGraphics::ISRenderResource* mResource = nullptr;
		SakuraFrameGraph* pFrameGraph = nullptr;
	};
}