#include "SDx12ImGuiDebugger.h"
#if defined(_AMD64_)
#include "Common/Thirdparty/Imgui/imgui.h"
#include "Common/Thirdparty/Imgui/imgui_impl_win32.h"
#include "Common/Thirdparty/Imgui/imgui_impl_dx12.h"
#endif
#include <tchar.h>
#include <Common/Thirdparty/Imgui/imnodes.h>
#include "../../FrameGraph/SakuraFrameGraph.h"
static int const NUM_FRAMES_IN_FLIGHT = 3;

bool SGraphics::SDx12ImGuiDebugger::Initialize(HWND hwnd, ID3D12Device* d3dDevice, ID3D12DescriptorHeap* d3dSrvDescHeap)
{
#if defined(_AMD64_)
	srvDescHeap = d3dSrvDescHeap;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.WantCaptureMouse = true;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(d3dDevice, NUM_FRAMES_IN_FLIGHT,
		DXGI_FORMAT_R8G8B8A8_UNORM, d3dSrvDescHeap,
		d3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		d3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
	imnodes::Initialize();
#endif
	return true;
}

void SGraphics::SDx12ImGuiDebugger::Draw(ID3D12GraphicsCommandList* cmdList, SakuraFrameGraph* frameGraph)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{
		static float f = 0.0f;
		static int counter = 0;
		ImGui::Begin("Render Graph");   
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
		imnodes::BeginNodeEditor();
		imnodes::StyleColorsDark();
		for (size_t i = 0; i < frameGraph->RootNodes.size(); i++)
		{
			imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(108, 236, 165, 255));
			DrawPassNode(frameGraph->RootNodes[i], 0, i);
			imnodes::PopColorStyle();
			DrawPassNodeChain(frameGraph->RootNodes[i], 1, i);
		}
		// Draw BackBuffer
		int hardcoded_node_id = std::hash<std::string>{}("BackBuffer");
		imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(55, 55, 55, 255));
		imnodes::SetNodeName(hardcoded_node_id, "BackBuffer");
		imnodes::BeginNode(hardcoded_node_id);
		imnodes::BeginInputAttribute(std::hash<std::string>{}("BackBufferInput"));
		ImGui::Text("BackBufferInput");
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		imnodes::EndAttribute();

		if (bFirstDraw)
		{
			imnodes::SetNodePos(hardcoded_node_id, ImVec2(200.f * MaxLayer + 60.f, 220.f));
		}
		imnodes::EndNode();
		imnodes::PopColorStyle();

		for (int i = 0; i < nodeLinks.size(); i++)
		{
			const std::pair<int, int> p = nodeLinks[i];
			imnodes::Link(i, p.first, p.second);
		}
		imnodes::PushColorStyle(imnodes::ColorStyle_Link, IM_COL32(255, 0, 0, 255));
		for (int t = 0; t < purePassLinks.size(); t++)
		{
			const std::pair<int, int> p = purePassLinks[t];
			imnodes::Link(nodeLinks.size() + t, p.first, p.second);
		}
		imnodes::PopColorStyle();
		imnodes::PushColorStyle(imnodes::ColorStyle_Link, IM_COL32(0, 0, 255, 255));
		for (int t = 0; t < endLinks.size(); t++)
		{
			const std::pair<int, int> p = endLinks[t];
			imnodes::Link(purePassLinks.size() + nodeLinks.size() + t, p.first, p.second);
		}
		imnodes::PopColorStyle();
		imnodes::PushColorStyle(imnodes::ColorStyle_Link, IM_COL32(255, 0, 255, 255));
		for (int t = 0; t < tracerLinks.size(); t++)
		{
			const std::pair<int, int> p = tracerLinks[t];
			imnodes::Link(purePassLinks.size() + nodeLinks.size() + endLinks.size() + t, p.first, p.second);
		}
		imnodes::PopColorStyle();

		imnodes::EndNodeEditor();
		ImGui::End();
	}
	cmdList->SetDescriptorHeaps(1, &srvDescHeap);
	ImGui::Render();
	bFirstDraw = false;
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
}

void SGraphics::SDx12ImGuiDebugger::DrawPassNode(SFG_PassNode* ToDraw, float layer, float neighber)
{
	auto PassNode = ToDraw;
	if (PassNode->ref == 0)
	{
		imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(245, 180 * 0.72, 155 * 0.69, 255));
		if (bFirstDraw)
		{
			std::pair<int, int> _pair;
			_pair.first = std::hash<std::string>{}(PassNode->mName + "Output");
			_pair.second = std::hash<std::string>{}("BackBufferInput");
			endLinks.push_back(_pair);
		}
	}
	int hardcoded_node_id = std::hash<std::string>{}(PassNode->mName);
	imnodes::SetNodeName(hardcoded_node_id, PassNode->mName.c_str());
	imnodes::BeginNode(hardcoded_node_id);
	if (bFirstDraw)
	{
		imnodes::SetNodePos(hardcoded_node_id, ImVec2(200.f * layer + 200.f, 
			120.f + 
			50 * ((PassNode->OutputResources.size() > PassNode->InputResources.size()) ? 
				PassNode->OutputResources.size() : PassNode->InputResources.size())
			* neighber));
	}
	ImGui::Text(("refs: " + std::to_string(PassNode->ref)).c_str());
	for (size_t j = 0; j < PassNode->InputResources.size(); j++)
	{
		imnodes::BeginInputAttribute(std::hash<std::string>{}(PassNode->mName + PassNode->InputResources[j].name + "Input"));
		ImGui::Text(PassNode->InputResources[j].name.c_str());
		imnodes::EndAttribute();
		if (bFirstDraw && PassNode->InputResources[j].writer != "null")
		{
			std::pair<int, int> _pair;
			_pair.first = std::hash<std::string>{}(PassNode->InputResources[j].writer + PassNode->InputResources[j].name + "Output");
			_pair.second = std::hash<std::string>{}(PassNode->mName + PassNode->InputResources[j].name + "Input");
			if (PassNode->InputResources[j].name == LifeTracer)
				tracerLinks.push_back(_pair);
			else
				nodeLinks.push_back(_pair);
		}
	}
	// No resource connected, add pass direct connect
	for (size_t j = 0; j < PassNode->PurePassPrevs.size(); j++)
	{
		imnodes::PushColorStyle(imnodes::ColorStyle_Pin, IM_COL32(255, 0, 0, 255));
		imnodes::BeginInputAttribute(std::hash<std::string>{}(PassNode->PurePassPrevs[j]->mName + "Input"));
		ImGui::Text(PassNode->PurePassPrevs[j]->mName.c_str());
		imnodes::EndAttribute();
		imnodes::PopColorStyle();
		if (bFirstDraw)
		{
			std::pair<int, int> _pair;
			_pair.first = std::hash<std::string>{}(PassNode->PurePassPrevs[j]->mName + "Output");
			_pair.second = std::hash<std::string>{}(PassNode->PurePassPrevs[j]->mName + "Input");
			purePassLinks.push_back(_pair);
		}
	}

	for (size_t j = 0; j < PassNode->OutputResources.size(); j++)
	{
		imnodes::BeginOutputAttribute(std::hash<std::string>{}
		(PassNode->OutputResources[j].writer + PassNode->OutputResources[j].name + "Output"));
		ImGui::Indent(40);
		ImGui::Text(PassNode->OutputResources[j].name.c_str());
		imnodes::EndAttribute();
	}

	imnodes::PushColorStyle(imnodes::ColorStyle_Pin, IM_COL32(255, 0, 0, 255));
	imnodes::BeginOutputAttribute(std::hash<std::string>{}(PassNode->mName + "Output"));
	ImGui::Indent(40);
	ImGui::Text(PassNode->mName.c_str());
	imnodes::EndAttribute();
	imnodes::PopColorStyle();
	imnodes::EndNode();
	if (PassNode->ref == 0)
		imnodes::PopColorStyle();
}

void SGraphics::SDx12ImGuiDebugger::DrawPassNodeSons(SFG_PassNode* ToDraw, float layer, float neighber)
{
	for (size_t j = 0; j < ToDraw->Nexts.size(); j++)
	{
		DrawPassNode(ToDraw->Nexts[j], layer, neighber++);
	}
}

void SGraphics::SDx12ImGuiDebugger::DrawPassNodeChain(SFG_PassNode* ToDraw, float layer, float neighber)
{
	DrawPassNodeSons(ToDraw, layer + 1, neighber);
	for (size_t i = 0; i < ToDraw->Nexts.size(); i++)
	{
		DrawPassNodeChain(ToDraw->Nexts[i], layer + 1, neighber + i);
	}
	if (layer + 1 > MaxLayer) MaxLayer = layer + 1;
}

