#pragma once
#include <windef.h>
#include <d3d12.h>
#include <utility>
#include <vector>
#include <string>

namespace SGraphics
{
	class SakuraFrameGraph;
	class SFG_PassNode;
}

namespace SGraphics
{
	class SDx12ImGuiDebugger 
	{
	public:
		bool Initialize(HWND hwnd, ID3D12Device* d3dDevice, ID3D12DescriptorHeap* d3dSrvDescHeap);
		void Draw(ID3D12GraphicsCommandList* cmdList, SakuraFrameGraph* frameGraph);

		ID3D12DescriptorHeap* srvDescHeap;
	private:
		void DrawPassNode(SFG_PassNode* ToDraw, float layer, float neighber);
		void DrawPassNodeSons(SFG_PassNode* ToDraw, float layer, float neighber);
		void DrawPassNodeChain(SFG_PassNode* ToDraw, float layer, float neighber);
		float MaxLayer = 0.f;
		std::vector<std::pair<int, int>> nodeLinks;
		std::vector<std::pair<int, int>> purePassLinks;
		std::vector<std::pair<int, int>> endLinks;
		std::vector<std::pair<int, int>> tracerLinks;
		std::string LifeTracer = "GBufferRTName1";
		bool bFirstDraw = true;
	};
}