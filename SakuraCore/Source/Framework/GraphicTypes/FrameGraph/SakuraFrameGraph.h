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

namespace SGraphics
{
	class SakuraFrameGraph : SImplements SakuraCore::IRuntimeModule
	{
	public:
		SakuraFrameGraph()
			:IRuntimeModule()
		{

		}
		SakuraFrameGraph(const SakuraFrameGraph& rhs) = delete;
		SakuraFrameGraph& operator=(const SakuraFrameGraph& rhs) = delete;

	public:
		// Initialize(Setup phase)
		virtual bool Initialize() override;
		virtual void Finalize() override;
		// Execute in Sakura Interface.
		virtual void Tick(double deltatime) override;
		// Compile phase
		virtual bool Compile();
	public:
		// Create node Interfaces:
		__forceinline decltype(PassTokenPtr) GetNamedRenderPass(const std::string& registName)
		{
			return (decltype(PassTokenPtr))mNamedPassNodes[registName]->GetPass();
		}

		template<typename T,
			typename std::enable_if<std::is_convertible<T*, decltype(PassTokenPtr)>::value>::type * = nullptr>
		__forceinline T* GetNamedRenderPass(const std::string& registName)
		{
#if defined(DEBUG) || defined(_DEBUG)
			return dynamic_cast<T*>(GetNamedRenderPass(registName));
#else
			return (T*)mNamedPassNodes[registName].get();
#endif
		}

		template<typename PassClass, typename... Params,
			typename std::enable_if<std::is_convertible<PassClass*, decltype(PassTokenPtr)>::value>::type * = nullptr>
		__forceinline decltype(PassTokenPtr) RegistNamedPassNode(const std::string& registName, Params... params)
		{
#if defined(DEBUG) || defined(_DEBUG)
			if (mNamedPassNodes.find(registName) != mNamedPassNodes.end())
				return mNamedPassNodes[registName]->GetPass();
			else
#endif
			{
				auto PassNode = std::make_unique<SFG_PassNode>();
				PassNode->Create<PassClass>(params...);
				mNamedPassNodes[registName] = std::move(PassNode);
				return mNamedPassNodes[registName]->GetPass();
			}
		}
	private:
		std::unordered_map<std::string, std::unique_ptr<SFG_PassNode>> mNamedPassNodes;
	};
}