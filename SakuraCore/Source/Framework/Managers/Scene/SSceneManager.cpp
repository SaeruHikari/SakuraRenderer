#include "SSceneManager.h"
#include "..\..\GraphicTypes\GraphicsCommon\UploadVertices.h"
#include "..\Graphics\D3D12\SDxRendererGM.h"

SakuraCore::SSceneManager::SSceneManager()
{
	pSceneManager = this;
	renderScene = std::make_unique<SRenderScene>();
	renderScene->Initialize();
}

SIndex SakuraCore::SSceneManager::RegistMesh(SStaticMesh* data, std::string matname)
{
	auto gmng = (SDxRendererGM*)pGraphicsManager;
	ID3D12Device* device = gmng->GetDevice();
	// Reset the command list to prep for initialization commands.
	ID3D12GraphicsCommandList* cmdList = gmng->GetDirectCmdList();
	ThrowIfFailed(cmdList->Reset(gmng->GetAlloc(), nullptr));

	auto _data = data->GetMeshData();

	SubmeshGeometry quadSubmesh;
	quadSubmesh.IndexCount = (UINT)_data->Indices32.size();
	quadSubmesh.StartIndexLocation = 0;
	quadSubmesh.BaseVertexLocation = 0;

	std::vector<StandardVertex> vertices(quadSubmesh.IndexCount);
	for (int i = 0; i < _data->Vertices.size(); ++i)
	{
		vertices[i].Pos = _data->Vertices[i].Position;
		vertices[i].TexC = _data->Vertices[i].TexC;
		vertices[i].Normal = _data->Vertices[i].Normal;
		vertices[i].TexC = _data->Vertices[i].TexC;
		vertices[i].Tangent = _data->Vertices[i].TangentU;
	}
	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(_data->GetIndices16()), std::end(_data->GetIndices16()));

	auto geo = new Dx12MeshGeometry();
	geo->Name = data->GetID();

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(StandardVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	// Create StandardVertex Buffer Blob
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
		cmdList, vertices.data(), vbByteSize, geo->VertexBufferUploader);
	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device,
		cmdList, indices.data(), ibByteSize, geo->IndexBufferUploader);
	geo->VertexByteStride = sizeof(StandardVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;
	geo->DrawArgs[geo->Name] = quadSubmesh;

	auto srItem = std::make_unique<SRenderItem>();
	auto rItem = &srItem->dxRenderItem;
	rItem->World = MathHelper::Identity4x4();
	rItem->TexTransform = MathHelper::Identity4x4();
	rItem->ObjCBIndex = gmng->CBIndex++;
	rItem->Geo = geo;
	rItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rItem->IndexCount = rItem->Geo->DrawArgs[geo->Name].IndexCount;
	rItem->StartIndexLocation = rItem->Geo->DrawArgs[geo->Name].StartIndexLocation;
	rItem->BaseVertexLocation = rItem->Geo->DrawArgs[geo->Name].BaseVertexLocation;
	rItem->NumFramesDirty = 3;
	gmng->mRenderLayers[SRenderLayers::E_Opaque].push_back(rItem);
	gmng->mAllRitems.push_back(std::move(std::unique_ptr<SDxRenderItem>(rItem)));
	renderScene->OpaqueRItems.push_back(std::move(srItem));
	gmng->mMaterials[matname] = &GetMaterial(matname)->data;
	rItem->Mat = gmng->mMaterials[matname];

	ThrowIfFailed(cmdList->Close());
	ID3D12CommandList* cmdsList0[] = { cmdList };
	gmng->GetQueue()->ExecuteCommandLists(_countof(cmdsList0), cmdsList0);
	gmng->FlushCommandQueue();
	return renderScene->OpaqueRItems.size() - 1;
}

SIndex SakuraCore::SSceneManager::RegistOpaqueMat(SMaterial* material, std::string name)
{
	auto unique = std::unique_ptr<SMaterial>(material);
	renderScene->OpaqueMaterials[name] = std::move(unique);

	return renderScene->OpaqueMaterials.size() - 1;
}

SGraphics::SRenderItem* SakuraCore::SSceneManager::GetRenderItem(SIndex index)
{
	return renderScene->OpaqueRItems[index].get();
}

SGraphics::SMaterial* SakuraCore::SSceneManager::GetMaterial(std::string Name)
{
	return renderScene->OpaqueMaterials[Name].get();
}

