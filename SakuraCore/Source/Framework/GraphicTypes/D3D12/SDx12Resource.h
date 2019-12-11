#pragma once
#include "..\GraphicsInterface\ISGPUResource.h"
#include <wrl\client.h>
#include <d3d12.h>

namespace SGraphics
{
	class SDx12Resource : public ISGPUResource
	{
	public:
		ID3D12Resource* Resource()
		{
			return mResource.Get();
		}
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
	};
}