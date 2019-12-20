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
		friend class SDx12ImGuiDebugger;
		friend class SGraphics::SakuraFrameGraph;
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
		SFG_ResourceHandle GetOutput(const std::string& resourceName);
	public:
		virtual void Setup();
		// Draw / 
		virtual void Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SFrameResource* frameResource);
		virtual void Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SFrameResource* frameResource, SGraphics::ISRenderTarget* backbuffer);
	public:
		void ConfirmInput_Internal(const std::vector<std::string>& ResourcesIn);

		template<typename... Resources>
		void ConfirmInput(Resources... resources)
		{
			InputResources.clear();
			InputResources.reserve(sizeof...(resources));
			ConfirmInput_Internal(resources...);
		}

		template<typename... Resources>
		void ConfirmOutput(Resources... resources)
		{
			OutputResources.clear();
			OutputResources.reserve(sizeof...(resources));
			ConfirmOutput_Internal(resources...);
		}
	private:
		template<typename Resource,
			typename std::enable_if<std::is_convertible<Resource*, std::string*>::value>::type * = nullptr>
			void ConfirmInput_Internal(const Resource& resource)
		{
			SFG_ResourceHandle handle;
			handle.name = resource;
			InputResources.push_back(handle); 
		}

		template<typename... Resources, typename Resource0, typename Resource1>
			void ConfirmInput_Internal(Resource0 resource0, Resource1 resource1, Resources... resources)
		{
			ConfirmInput_Internal(resource0);
			ConfirmInput_Internal(resource1, resources...);
		}

		template<typename Resource,
			typename std::enable_if<std::is_convertible<Resource*, SFG_ResourceHandle*>::value>::type * = nullptr>
			void ConfirmInput_Internal(const Resource& resource)
		{
			InputResources.push_back(resource);
		}

		template<typename Pass,
			typename std::enable_if<std::is_convertible<Pass*, SFG_PassNode*>::value>::type * = nullptr>
		void ConfirmInput_Internal(Pass* pass)
		{
			AddInputPass(pass);
			if(std::find(PurePassPrevs.begin(), PurePassPrevs.end(), pass) == PurePassPrevs.end())
				PurePassPrevs.push_back(pass);
		}

		template<typename... Resources, typename Resource0, typename Resource1,
			typename std::enable_if<std::is_convertible<Resource1*, std::string*>::value>::type * = nullptr>
			void ConfirmOutput_Internal(Resource0 resource0, Resource1 resource1, Resources... resources)
		{
			ConfirmOutput_Internal(resource0);
			ConfirmOutput_Internal(resource1, resources...);
		}
		template<typename Resource,
			typename std::enable_if<std::is_convertible<Resource*, std::string*>::value>::type * = nullptr>
			void ConfirmOutput_Internal(Resource resource)
		{
			SFG_ResourceHandle handle;
			handle.name = resource;
			handle.writer = mName;
			OutputResources.push_back(handle);
		}
	protected:
		__forceinline void AddInputPass(SFG_PassNode* inputpass)
		{
			if (inputpass->Nexts.size() > 0)
			{ 
				if (find(inputpass->Nexts.begin(), inputpass->Nexts.end(), this) != inputpass->Nexts.end())
					return;
			}
			if (Prevs.size() > 0)
			{
				if (find(Prevs.begin(), Prevs.end(), inputpass) != Prevs.end())
					return;
			}
			inputpass->Nexts.push_back(this);
			Prevs.push_back(inputpass);
			inputpass->ref++;
		}
		SFG_PassNode* GetNamedNode(std::string name);
		__forceinline bool Compiled() { return bCompiled; }
		__forceinline bool HasOutput(const std::string& name)
		{
			for (size_t i = 0; i < OutputResources.size(); i++)
			{
				if (OutputResources[i].Is(name))
					return true;
			}
			return false;
		}
		std::string mName = "NULL";
		size_t readyCount = 0;
		size_t ref = 0;
		std::vector<SFG_ResourceHandle> InputResources;
		std::vector<SFG_ResourceHandle> OutputResources;
		std::vector<SFG_PassNode*> Nexts;
		std::vector<SFG_PassNode*> Prevs;
		std::vector<SFG_PassNode*> PurePassPrevs;
		std::unique_ptr<SGraphics::SRHIPass> mPass;
		SakuraFrameGraph* pFrameGraph = nullptr;
		bool bCompiled = false;
	};
}