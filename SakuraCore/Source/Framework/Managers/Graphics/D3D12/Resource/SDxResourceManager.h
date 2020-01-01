#pragma once
#include "../../CommonInterface/Resource/SakuraGraphicsResourceManagerBase.h"
#include "Framework/GraphicTypes/D3D12/SDescriptorHeap.hpp"
#include "Framework/GraphicTypes/D3D12/Utils/MeshImporter.h"

class SFrameResource;
namespace SGraphics
{
	class ISDx12RenderTarget;

	// CB: In Frame Resources
	// Here: Create Rtv Srv Descriptors
	class SDxResourceManager : public SakuraGraphicsResourceManagerBase
	{
	public:
		SDxResourceManager() = default;
		SDxResourceManager(Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory,
			Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice,
			std::shared_ptr<SFence> fence,
			std::shared_ptr<SDx12DeviceInformation> deviceInformation,
			std::shared_ptr<SDx12GraphicsStates> graphicsConfs);
		SDxResourceManager(const SDxResourceManager& rhs) = delete;
		SDxResourceManager& operator=(const SDxResourceManager&) = delete;
		~SDxResourceManager();
	protected:
		void FlushCommandQueue();
	public:
		// Initialize function.
		virtual bool Initialize();
		// Finalize function.
		virtual void Finalize();
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime);
		virtual SDescriptorHeap* GetOrAllocDescriptorHeap(std::string name,
			UINT descriptorSize = 0, 
			D3D12_DESCRIPTOR_HEAP_DESC desc = {});
		// Get Textures
		virtual ISRenderTarget* CreateNamedRenderTarget(std::string resgistName, ISRenderTargetProperties rtProp,
			std::string targetSrvHeap, std::string targetRtvHeap, SRHIResource* resource = nullptr) override;
		virtual ISRenderTarget* CreateNamedRenderTarget(std::string registName,
			ISRenderTargetProperties rtProp, SRHIResource* resource, SResourceHandle srvHandle, SResourceHandle rtvHandle) override;

		Dx12MeshGeometry* RegistGeometry(const std::string& GeoName,
			std::string FilePath, HikaD3DUtils::ESupportFileForm FileForm);

		template<typename Vertex, typename Index>
		Dx12MeshGeometry* RegistGeometry(const std::string& geometryName, const std::string& drawArgName,
			const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
		{
			ThrowIfFailed(mDirectCmdListAlloc->Reset());
			ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
			SubmeshGeometry subMesh;
			subMesh.IndexCount = indices.size();
			subMesh.VertexCount = vertices.size();
			int startIndex = 0;
			int baseVertex = 0;
			if (mGeometries.find(geometryName) != mGeometries.end())
			{
				auto drawArg = mGeometries[geometryName]->DrawArgs.begin();
				while (drawArg != mGeometries[geometryName]->DrawArgs.end())
				{
					startIndex = drawArg->second.StartIndexLocation > startIndex ?
						drawArg->second.StartIndexLocation + drawArg->second.IndexCount : startIndex;
					baseVertex = drawArg->second.BaseVertexLocation > baseVertex ?
						drawArg->second.BaseVertexLocation + drawArg->second.VertexCount : baseVertex;
				}
			}
			subMesh.StartIndexLocation = startIndex;
			subMesh.BaseVertexLocation = baseVertex;

			auto geo = std::make_unique<Dx12MeshGeometry>();
			geo->Name = geometryName;

			const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
			const UINT ibByteSize = (UINT)indices.size() * sizeof(Index);

			// Create StandardVertex Buffer Blob
			ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
			CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
			ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
			CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

			geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);
			geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
				mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);
			geo->VertexByteStride = sizeof(Vertex);
			geo->VertexBufferByteSize = vbByteSize;
			geo->IndexFormat = DXGI_FORMAT_R16_UINT;
			geo->IndexBufferByteSize = ibByteSize;

			// Execute the initialization commands.
			ThrowIfFailed(mCommandList->Close());
			ID3D12CommandList* cmdsList0[] = { mCommandList.Get() };
			mCommandQueue->ExecuteCommandLists(_countof(cmdsList0), cmdsList0);
			FlushCommandQueue();

			geo->DrawArgs[drawArgName] = subMesh;
			mGeometries[geometryName] = std::move(geo);
			return mGeometries[geometryName].get();
		}
		auto GetGeometry(const std::string& name)
		{
			return mGeometries[name].get();
		}
	protected:
		std::unordered_map<std::string, std::unique_ptr<Dx12MeshGeometry>> mGeometries;
	protected:
		Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;
		std::shared_ptr<SFence> mFence;
		std::shared_ptr<SDx12DeviceInformation> mDeviceInformation;
		std::shared_ptr<SDx12GraphicsStates> mGraphicsConfs;
		
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
		
		std::map<std::string, std::unique_ptr<SDescriptorHeap>> mDescriptorHeaps;

		bool InitD3D12Device();
		void CreateCommandObjects();
	protected:
		virtual ISTexture* LoadTexture(std::wstring Filename, std::string textName) override;
	};

}