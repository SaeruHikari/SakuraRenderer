/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.12.14
Description:				 Sakura FrameGraph (DAG) class.
Details:              A DAG that clearly knows global pass and resource usage.
*******************************************************************************************/
#pragma once
#include "Interface\IRuntimeModule.h"
#include "..\GraphicsCommon\GraphicsConfigs.h"
#include "Framework/GraphicTypes/D3D12/SDx12Pass.hpp"
#include "Framework/GraphicTypes/FrameGraph/SFG_PassNode.h"
#include "SFG_ResourceNode.h"

namespace SGraphics
{
	class SakuraGraphicsResourceManagerBase;
	class ISRenderTarget;
}

namespace SGraphics
{
	// Desired Pattern:
	//     pFrameGraph->RegistNamedPassNode<SSpecPass>
	//             ("Name_ThisNode", params)
	//     pFrameGraph->GetNamedPassNode("Name")->ConfirmInOut<resourceIn, resourceOut>("Name_ResourceIn"[], "Names_resourceOut"[]);
	//     pFrameGraph->GetNamedPassNode("Name")->After("Name");
	//     Compile: Construct DAG
	class SakuraFrameGraph : SImplements SakuraCore::IRuntimeModule
	{
	public:
		SakuraFrameGraph(SakuraGraphicsResourceManagerBase* resourceManager)
			:IRuntimeModule() 
		{
			pGraphicsResourceManager = resourceManager;
		}
		SakuraFrameGraph(const SakuraFrameGraph& rhs) = delete;
		SakuraFrameGraph& operator=(const SakuraFrameGraph& rhs) = delete;

	public:
		// Initialize
		virtual bool Initialize() override;
		virtual void Finalize() override;
		// Execute in Sakura Interface.
		virtual void Tick(double deltatime) override;
		// Setup phase
		virtual void Setup();
		// Compile phase
		virtual bool Compile();
		virtual SResourceHandle* GetResourceHandle(std::string resourceName);
		virtual SResourceHandle* GetRenderTargetHandle(std::string rtName);
		virtual void Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SResourceCPUHandle* backbuffer, SFrameResource* frameResource);
	private:
		std::unordered_map<std::string, std::unique_ptr<SFG_PassNode>> mNamedPassNodes;
		std::unordered_map<std::string, std::unique_ptr<SFG_ResourceNode>> mNamedResourceNodes;
		inline static SakuraGraphicsResourceManagerBase* pGraphicsResourceManager = nullptr;

	public:
		// Create node Interfaces:
		__forceinline SRHIPass* GetNamedRenderPass(const std::string& registName)
		{
			if (mNamedPassNodes.find(registName) != mNamedPassNodes.end())
				return (SRHIPass*)mNamedPassNodes[registName]->GetPass();
			else return nullptr;
		}

		__forceinline SFG_PassNode* GetNamedPassNode(const std::string& registName)
		{
			if (mNamedPassNodes.find(registName) != mNamedPassNodes.end())
				return mNamedPassNodes[registName].get();
			else return nullptr;
		}

		__forceinline SFG_ResourceNode* GetNamedRenderResourceNode(const std::string& registName)
		{
			if (mNamedResourceNodes.find(registName) != mNamedResourceNodes.end())
				return mNamedResourceNodes[registName].get();
			else return nullptr;
		}

		__forceinline ISRenderResource* GetNamedRenderResource(const std::string& registName)
		{
			if (mNamedResourceNodes.find(registName) != mNamedResourceNodes.end())
				return mNamedResourceNodes[registName]->GetResource();
			else return nullptr;
		}
	public:
		template<typename T,
			typename std::enable_if<std::is_convertible<T*, ISRenderResource*>::value>::type * = nullptr>
		__forceinline T* GetNamedRenderResource(const std::string& registName)
		{
#if defined(DEBUG) || defined(_DEBUG)
			return dynamic_cast<T*>(GetNamedRenderResource(registName));
#else
			return (T*)mNamedResourceNodes[registName]->GetResource();
#endif
		}

		template<typename T,
			typename std::enable_if<std::is_convertible<T*, SRHIPass*>::value>::type * = nullptr>
			__forceinline T* GetNamedRenderPass(const std::string& registName)
		{
#if defined(DEBUG) || defined(_DEBUG)
			return dynamic_cast<T*>(GetNamedRenderPass(registName));
#else
			return (T*)mNamedPassNodes[registName]->GetPass();
#endif
		}

	public:
		// Register DAG Node for a Pass.
		template<typename PassClass, typename... Params,
			typename std::enable_if<std::is_convertible<PassClass*, SRHIPass*>::value>::type * = nullptr>
		__forceinline PassClass* RegistNamedPassNode(const std::string& registName, Params... params)
		{
#if defined(DEBUG) || defined(_DEBUG)
			if (mNamedPassNodes.find(registName) != mNamedPassNodes.end())
				return (PassClass*)(mNamedPassNodes[registName]->GetPass());
			else
#endif
			{
				mNamedPassNodes[registName] = std::make_unique<SFG_PassNode>(this, registName);
				mNamedPassNodes[registName]->Create<PassClass>(params...);
				return (PassClass*)mNamedPassNodes[registName]->GetPass();
			}
		}

		// Register DAG Node for a resource.
		template<typename ResourceClass,
			typename std::enable_if<std::is_convertible<ResourceClass*, ISTexture*>::value>::type * = nullptr>
		__forceinline ResourceClass* RegistNamedResourceNode(const std::wstring& fileName, const std::string& registName, SRHIResource* resourceIn = nullptr)
		{
			ISRenderResource* resource = nullptr;
			if (resourceIn == nullptr)
				resource = pGraphicsResourceManager->LoadTexture(fileName, registName);
			else
				resource = new SD3DTexture(registName, resourceIn);
			auto ResourceNode = std::make_unique<SFG_ResourceNode>(this, resource);
			mNamedResourceNodes[registName] = std::move(ResourceNode);
#if defined(DEBUG) || defined(_DEBUG)
			return dynamic_cast<ResourceClass*>(mNamedResourceNodes[registName]->GetResource());
#else
			return (ResourceClass*)mNamedResourceNodes[registName]->GetResource();
#endif
		}


		// Register DAG Node for a resource.
		template<typename ResourceClass, typename... Params,
			typename std::enable_if<std::is_convertible<ResourceClass*, ISRenderTarget*>::value>::type * = nullptr>
		__forceinline ResourceClass* RegistNamedResourceNode(const std::string& registName, Params... params)
		{
			if (mNamedResourceNodes.find(registName) != mNamedResourceNodes.end())
#if defined(DEBUG) || defined(_DEBUG)
				return dynamic_cast<ResourceClass*>(mNamedResourceNodes[registName]->GetResource());
#else
				return (ResourceClass*)(mNamedResourceNodes[registName]->GetResource());
#endif
			else
			{
				auto resource = pGraphicsResourceManager->CreateNamedRenderTarget(registName, params...);
				auto ResourceNode = std::make_unique<SFG_ResourceNode>(this, resource);
				mNamedResourceNodes[registName] = std::move(ResourceNode);
#if defined(DEBUG) || defined(_DEBUG)
				return dynamic_cast<ResourceClass*>(mNamedResourceNodes[registName]->GetResource());
#else
				return (ResourceClass*)mNamedResourceNodes[registName]->GetResource();
#endif
			}
		}

	};
}