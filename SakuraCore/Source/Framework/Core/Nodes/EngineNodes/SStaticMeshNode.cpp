#include "SStaticMeshNode.hpp"

REFLECTION_EDITOR
(
	registration::class_<SStaticMeshNode>("SStaticMeshNode")
	.constructor<>()
	.property("StaticMeshes", &SStaticMeshNode::GetSubMeshes, &SStaticMeshNode::SetSubMeshes)
	.property("SubMesh", &SStaticMeshNode::testsubmesh);

	registration::class_<SStaticSubmesh>("SStaticSubmesh")
	.property("Name", &SStaticSubmesh::GetName, &SStaticSubmesh::SetName)
	.property("MaterialName", &SStaticSubmesh::GetMaterialName, &SStaticSubmesh::SetMaterialName)
	.constructor<>();
)

void SEngine::SStaticMeshNode::UpdateTransform()
{
	for (auto i = 0u; i < submeshes.size(); i++)
	{
		if (auto renderItem = submeshes[i]->GetRenderItem())
		{
			renderItem->dxRenderItem.PrevWorld = renderItem->dxRenderItem.World;
			auto par = parentNode;
			auto Scale = GetWorldScale();
			XMStoreFloat4x4(&(renderItem->dxRenderItem.World),
				XMMatrixScaling(GetWorldScale().x, GetWorldScale().y, GetWorldScale().z) *
				XMMatrixTranslation(NodeTransform.Location.x + NodeTransform.Origin.x,
					NodeTransform.Location.y + NodeTransform.Origin.y,
					NodeTransform.Location.z + NodeTransform.Origin.z));
			renderItem->dxRenderItem.NumFramesDirty = gNumFrameResources;
		}
	}
}

void SEngine::SStaticMeshNode::Tick(double deltaTime)
{
	SakuraSceneNode::Tick(deltaTime);
	UpdateTransform();
}
