/*****************************************************************************************
							 Copyrights   SaeruHikari
CreateDate:							2019.10.21
Description:	          Pool of d3d12 descriptor heaps.
Details:
*******************************************************************************************/
#pragma once
#include "Interface/ISObject.h"
#include "Framework/GraphicTypes/D3D12/D3DCommon.h"
#include "../GraphicsCommon/GraphicsConfigs.h"
#include "../GraphicsInterface/ISRenderResource.h"

namespace SGraphics
{
	struct SDescriptorHeap;
	extern const size_t gPoolPageSize;
	struct DescriptorHandleCouple : public SGraphics::SResourceHandle
	{
		SDescriptorHeap* fromHeap = nullptr;
	};
	struct SDescriptorHeap
	{
	public:
		SDescriptorHeap() = default;
		SDescriptorHeap(ID3D12Device* m_device, size_t descriptorSize, D3D12_DESCRIPTOR_HEAP_DESC desc)
			:heapDescriptorSize(descriptorSize), filledNum(0)
		{
			ThrowIfFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
		}
		inline CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUtDescriptorHandle(size_t index) const
		{
			auto _handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart());
			_handle.Offset(index, heapDescriptorSize);
			return _handle;
		}
		inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUtDescriptorHandle(size_t index) const
		{
			auto _handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
			_handle.Offset(index, heapDescriptorSize);
			return _handle;
		}
		inline DescriptorHandleCouple GetAvailableHandle()
		{
			DescriptorHandleCouple couple;
			couple.hCpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart()).Offset(filledNum, heapDescriptorSize);;
			couple.hGpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart()).Offset(filledNum, heapDescriptorSize);
			couple.indexOnHeap = filledNum;
			couple.fromHeap = this;
			filledNum = filledNum + 1;
			return couple;
		}
		inline D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPUFromCPU(D3D12_CPU_DESCRIPTOR_HANDLE handle) const 
		{
			D3D12_GPU_DESCRIPTOR_HANDLE rebase;
			rebase.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr
				+ UINT64(handle.ptr - descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr);
			return rebase;
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPUFromGPU(D3D12_GPU_DESCRIPTOR_HANDLE handle) const
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rebase;
			rebase.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr
				+ UINT64(handle.ptr - descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr);
			return rebase;
		}
		inline ID3D12DescriptorHeap* DescriptorHeap() { return descriptorHeap.Get(); }
	protected:
		size_t heapDescriptorSize = 0;
		size_t filledNum = 0;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	};
}