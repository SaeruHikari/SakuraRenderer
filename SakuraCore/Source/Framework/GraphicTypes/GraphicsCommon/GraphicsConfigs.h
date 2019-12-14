#pragma once
#define REVERSE_Z
#define Sakura_D3D

namespace SGraphics
{
	class SDx12Pass;
	//class SakuraFrameGraph;
}

namespace SGraphics
{
#if defined(Sakura_D3D)
	inline static SDx12Pass* PassTokenPtr = nullptr;
#elif defined(Sakura_Vulkan)
	inline static SVulkanPass* PassTokenPtr = nullptr;
#else
	inline static void* PassTokenPtr = nullptr;
#endif
	//inline static std::unique_ptr<SakuraFrameGraph> pFrameGraph = nullptr;
}
