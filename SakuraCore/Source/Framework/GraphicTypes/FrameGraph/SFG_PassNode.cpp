#include "SFG_PassNode.h"
#include "SakuraFrameGraph.h"
#include "..\D3D12\SDx12Pass.hpp"
#include "..\GraphicsInterface\ISRenderTarget.h"

SGraphics::SFG_ResourceHandle SGraphics::SFG_PassNode::GetOutput(const std::string& resourceName)
{
	for (size_t i = 0; i < OutputResources.size(); i++)
	{
		if (OutputResources[i].name == resourceName)
			return OutputResources[i];
	}
	std::string msg = "PASS: " + mName + "DO NOT HAVE output resource called" + resourceName;
	assert(0 && msg.c_str());
	SFG_ResourceHandle handle;
	return handle;
}

void SGraphics::SFG_PassNode::Setup()
{
	if (InputResources.size() > 0)
	{
		std::vector<ID3D12Resource*> resources;
		resources.resize(InputResources.size());
		for (size_t i = 0; i < InputResources.size(); i++)
		{
			auto node = 
				pFrameGraph->GetNamedRenderResourceNode(InputResources[i]);
#if defined(DEBUG) || defined(_DEBUG)
			std::string err = "writer: " + InputResources[i].writer + " \nresource:" + InputResources[i].name + " NOT EXISTED";
			assert(node != nullptr && err.c_str());
#endif
			resources[i] = node->GetResource()->GetGPUResource();
		}
		GetPass()->Initialize(resources);
	}
	else
		GetPass()->Initialize();
}

void SGraphics::SFG_PassNode::Execute(SCommandList* cmdList, SResourceCPUHandle* dsv, SFrameResource* frameResource)
{
	ISRenderTarget** mvRtv = new ISRenderTarget*[OutputResources.size()];
	for (size_t i = 0; i < OutputResources.size(); i++)
	{
		mvRtv[i] = (ISRenderTarget*)pFrameGraph->GetNamedRenderResource(OutputResources[i].name);
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			pFrameGraph->GetNamedRenderResource(OutputResources[i].name)->GetGPUResource(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
	mPass->Execute(cmdList,
		dsv, 
		frameResource,
		mvRtv, OutputResources.size());
	for (size_t i = 0; i < OutputResources.size(); i++)
	{
		cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			pFrameGraph->GetNamedRenderResource(OutputResources[i].name)->GetGPUResource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
	}
	delete[] mvRtv;
}



void SGraphics::SFG_PassNode::Execute(SCommandList* cmdList, SResourceCPUHandle* dsv,
	SFrameResource* frameResource, ISRenderTarget* backbuffer)
{
	mPass->Execute(cmdList,
		dsv,
		frameResource,
		&backbuffer, 1);
}

void SGraphics::SFG_PassNode::ConfirmInput_Internal(const std::vector<std::string>& ResourcesIn)
{
	InputResources.resize(ResourcesIn.size());
	for (size_t i = 0; i < ResourcesIn.size(); i++)
	{
		InputResources[i].name = ResourcesIn[i];
	}
}

SGraphics::SFG_PassNode* SGraphics::SFG_PassNode::GetNamedNode(std::string name)
{
	return pFrameGraph->GetNamedPassNode(name);
}
