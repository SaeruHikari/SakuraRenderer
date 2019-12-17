#pragma once
#include "Interface/ISObject.h"
#include "Framework/GraphicTypes/GraphicsCommon/GraphicsConfigs.h"
#include "..\D3D12\FrameResource.h"
#include "SFG_ResourceHandle.h"

namespace SGraphics
{
	struct ISRenderPass;
	struct ISRenderTarget;
	class SakuraFrameGraph;
	class SakuraGraphicsResourceManagerBase;
}

namespace SGraphics
{
	class SFG_PassNode : SImplements SakuraCore::ISSilentObject
	{
	public:
		SFG_PassNode(SakuraFrameGraph* frameGraph, const std::string& name)
			:ISSilentObject(), mName(name)
		{
			pFrameGraph = frameGraph;
		}
		template<typename PassClass, typename... Params,
			typename std::enable_if<std::is_convertible<PassClass*, SRHIPass*>::value>::type * = nullptr>
		__forceinline void Create(Params... params)
		{
			mPass = std::make_unique<PassClass>(params...);
		}
		__forceinline auto GetPass()
		{
			return mPass.get();
		}

	public:
		virtual void Setup();
		// Draw / 
		virtual void Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SFrameResource* frameResource);
		virtual void Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SFrameResource* frameResource, SGraphics::ISRenderTarget* backbuffer);
	public:
		void ConfirmResourceInOut(const std::vector<std::string>& ResourcesIn, const std::vector<std::string>& ResourceOut);
		void ConfirmResourceInOut(const std::vector<std::string>& ResourcesIn);
		void ConfirmResourceOut(const std::vector<std::string>& ResourcesOut);

	protected:
		SFG_PassNode* GetNamedNode(std::string name);
		std::string mName = "NULL";
		std::vector<SFG_ResourceHandle> InputResources;
		std::vector<SFG_ResourceHandle> OutputResources;
		std::unique_ptr<SGraphics::SRHIPass> mPass;
		SakuraFrameGraph* pFrameGraph = nullptr;
	};
}