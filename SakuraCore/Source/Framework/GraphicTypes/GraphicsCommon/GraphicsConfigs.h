#pragma once
#define REVERSE_Z
#define Sakura_D3D

namespace SGraphics
{
	class SDx12Pass;
}

class ID3D12Resource;
class ID3D12GraphicsCommandList;
class D3D12_CPU_DESCRIPTOR_HANDLE;
class D3D12_GPU_DESCRIPTOR_HANDLE;

namespace SGraphics
{
#if defined(Sakura_D3D)
	typedef SDx12Pass SRHIPass;
	typedef ID3D12Resource SRHIResource;
	typedef ID3D12GraphicsCommandList SCommandList;
	typedef D3D12_CPU_DESCRIPTOR_HANDLE SResourceCPUHandle;
	typedef D3D12_GPU_DESCRIPTOR_HANDLE SResourceGPUHandle;
#elif defined(Sakura_Vulkan)
	inline static SVulkanPass* PassTokenPtr = nullptr;
#else
	inline static void* PassTokenPtr = nullptr;
#endif
}
