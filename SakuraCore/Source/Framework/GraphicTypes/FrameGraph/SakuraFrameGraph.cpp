#include "SakuraFrameGraph.h"
#include "..\..\Managers\Graphics\CommonInterface\Resource\SakuraGraphicsResourceManagerBase.h"


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

