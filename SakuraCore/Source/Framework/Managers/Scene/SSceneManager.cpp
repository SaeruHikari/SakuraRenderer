#include "SSceneManager.h"
#include "..\..\GraphicTypes\GraphicsCommon\UploadVertices.h"
#include "..\Graphics\D3D12\SDxRendererGM.h"

SakuraCore::SSceneManager::SSceneManager()
{
	pSceneManager = this;
	renderScene = std::make_unique<SRenderScene>();
	renderScene->Initialize();
}

SIndex SakuraCore::SSceneManager::RegistRenderItem(const std::string& geomName, const std::string& drawArgName,
	std::string matname, ERenderLayer renderLayer)
{
	auto gmng = (SDxRendererGM*)pGraphicsManager;
	auto srItem = std::make_unique<SRenderItem>();
	auto rItem = &srItem->dxRenderItem;
	rItem->World = MathHelper::Identity4x4();
	rItem->Mat = &GetMaterial(matname)->data;
	rItem->TexTransform = MathHelper::Identity4x4();
	rItem->ObjCBIndex = renderScene->mAllRItems.size();
	rItem->Geo = gmng->GetResourceManager()->GetGeometry(geomName);
	rItem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	rItem->IndexCount = rItem->Geo->DrawArgs[drawArgName].IndexCount;
	
	rItem->StartIndexLocation = rItem->Geo->DrawArgs[drawArgName].StartIndexLocation;
	rItem->BaseVertexLocation = rItem->Geo->DrawArgs[drawArgName].BaseVertexLocation;
	rItem->NumFramesDirty = 3;
	renderScene->mRenderLayers[renderLayer].push_back(srItem.get());
	renderScene->mAllRItems.push_back(std::move(srItem));
	return renderScene->mAllRItems.size() - 1;
}

SMaterial* SakuraCore::SSceneManager::RegistOpaqueMat(const std::string& name, OpaqueMaterial& opaqueMat)
{
	return renderScene->RegistOpaqueMaterial(name, opaqueMat);
}

SGraphics::SRenderItem* SakuraCore::SSceneManager::GetRenderItem(SIndex index)
{
	return renderScene->mAllRItems[index].get();
}

SGraphics::SMaterial* SakuraCore::SSceneManager::GetMaterial(std::string Name)
{
	return renderScene->OpaqueMaterials[Name].get();
}

