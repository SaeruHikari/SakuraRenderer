#include "SFG_PassNode.h"
#include "SakuraFrameGraph.h"
#include "..\D3D12\SDx12Pass.hpp"
#include "..\GraphicsInterface\ISRenderTarget.h"

void SGraphics::SFG_PassNode::Setup()
{
	if (InputResources.size() > 0)
	{
		std::vector<ID3D12Resource*> resources;
		resources.resize(InputResources.size());
		for (size_t i = 0; i < InputResources.size(); i++)
		{
			auto resourceNode = pFrameGraph->GetNamedRenderResourceNode(InputResources[i].name);
			resourceNode->AddRef(this);
			resources[i] = resourceNode->GetResource()->GetGPUResource();
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

void SGraphics::SFG_PassNode::ConfirmResourceInOut(const std::vector<std::string>& ResourcesIn, const std::vector<std::string>& ResourceOut)
{
	InputResources.resize(ResourcesIn.size());
	for (size_t i = 0; i < ResourcesIn.size(); i++)
	{
		InputResources[i].name = ResourcesIn[i];
		InputResources[i].version = 0;
	}
	OutputResources.resize(ResourceOut.size());
	for (size_t i = 0; i < ResourceOut.size(); i++)
	{
		OutputResources[i].name = ResourceOut[i];
		OutputResources[i].version = 0;
	}
}

void SGraphics::SFG_PassNode::ConfirmResourceOut(const std::vector<std::string>& ResourcesOut)
{
	OutputResources.resize(ResourcesOut.size());
	for (size_t i = 0; i < ResourcesOut.size(); i++)
	{
		OutputResources[i].name = ResourcesOut[i];
		OutputResources[i].version = 0;
	}
}

void SGraphics::SFG_PassNode::ConfirmResourceInOut(const std::vector<std::string>& ResourcesIn)
{
	InputResources.resize(ResourcesIn.size());
	for (size_t i = 0; i < ResourcesIn.size(); i++)
	{
		InputResources[i].name = ResourcesIn[i];
		InputResources[i].version = 0;
	}
}

SGraphics::SFG_PassNode* SGraphics::SFG_PassNode::GetNamedNode(std::string name)
{
	return pFrameGraph->GetNamedPassNode(name);
}
