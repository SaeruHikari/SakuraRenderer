#include "SakuraFrameGraph.h"


bool SGraphics::SakuraFrameGraph::Initialize()
{
	return true;
}

void SGraphics::SakuraFrameGraph::Finalize()
{

}

void SGraphics::SakuraFrameGraph::Tick(double deltatime)
{

}

void SGraphics::SakuraFrameGraph::Setup()
{
	auto iter = mNamedPassNodes.begin();
	while (iter != mNamedPassNodes.end())
	{
		iter->second->Setup();
		iter++;
	}
}

bool SGraphics::SakuraFrameGraph::Compile()
{
	auto iter = mNamedPassNodes.begin();
	while (iter != mNamedPassNodes.end())
	{
		Compile(iter->second.get());
		iter++;
	}
	RootNodes.clear();
	iter = mNamedPassNodes.begin();
	while (iter != mNamedPassNodes.end())
	{
		if (iter->second->Prevs.size() == 0)
			RootNodes.push_back(iter->second.get());
		iter++;
	}
	return true;
}

bool SGraphics::SakuraFrameGraph::Compile(SFG_PassNode* nodeToCompile)
{
	size_t InLength = nodeToCompile->InputResources.size();
	if (InLength > 0)
	{
		for (size_t i = 0; i < InLength; i++)
		{
			if (nodeToCompile->InputResources[i].writer == "NULL") continue;
			auto parent = GetNamedPassNode(nodeToCompile->InputResources[i].writer);
			if (parent == nullptr)
			{
				std::string err = nodeToCompile->InputResources[i].writer;
				err = "Pass node " + err + " not registered!";
				assert(0 && err.c_str());
				return false;
			}
			if (!parent->Compiled())
			{
				if (!Compile(parent))
				{
					std::string err = parent->mName;
					err = "Pass node " + err + " compile failed!";
					assert(0 && err.c_str());
					return false;
				}
				// parent has output of the resource.
				else if (parent->HasOutput(nodeToCompile->InputResources[i].name))
				{
					nodeToCompile->AddInputPass(parent);
				}
			}
		}
	}
	// Deal with outputs
	size_t OutLength = nodeToCompile->OutputResources.size();
	if (OutLength > 0)
	{
		for (size_t i = 0; i < nodeToCompile->OutputResources.size(); i++)
		{
			RegistNewVersionResourceNode(nodeToCompile->OutputResources[i]);
		}
	}

	return true;
}

SGraphics::SResourceHandle* SGraphics::SakuraFrameGraph::GetResourceHandle(std::string resourceName)
{
	return pGraphicsResourceManager->GetResourceHandle(resourceName);
}

SGraphics::SResourceHandle* SGraphics::SakuraFrameGraph::GetRenderTargetHandle(std::string rtName)
{
	return pGraphicsResourceManager->GetRenderTargetHandle(rtName);
}

void SGraphics::SakuraFrameGraph::Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SResourceCPUHandle* backbuffer, SFrameResource* frameResource)
{

}

void SGraphics::SakuraFrameGraph::RegistNewVersionResourceNode(const std::string& resource, const std::string& writer)
{
	SFG_ResourceHandle handle;
	handle.name = resource;
	if (mNamedResourceNodes.find(handle) != mNamedResourceNodes.end())
	{
		// Create
		auto res = mNamedResourceNodes[handle]->Duplicate();
		handle.writer = writer;
		mNamedResourceNodes[handle] = std::move(res);
	}
	else
	{
		std::string err = resource;
		err = "No registered resource called: " + err + " found in resource system! ";
		assert(0 && err.c_str());
	}
}

void SGraphics::SakuraFrameGraph::RegistNewVersionResourceNode(const SFG_ResourceHandle& newhandle)
{
	SFG_ResourceHandle handle;
	handle.name = newhandle.name;
	if (mNamedResourceNodes.find(handle) != mNamedResourceNodes.end())
	{
		// Create
		auto res = mNamedResourceNodes[handle]->Duplicate();
		handle.writer = newhandle.writer;
		mNamedResourceNodes[handle] = std::move(res);
	}
	else
	{
		std::string err = newhandle.name;
		err = "No registered resource called: " + err + " found in resource system! ";
		assert(0 && err.c_str());
	}
}

