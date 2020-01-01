#include "SceneImporter.h"
#include <Framework\Core\Nodes\SakuraSceneNode.hpp>

std::unique_ptr<SScene::SakuraSceneNode> SSceneImporter::ImportScene(std::string FilePath)
{
	auto SSceneRoot = std::make_unique<SScene::SakuraSceneNode>();
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
	// Cannot remove pivot points because the static mesh will be in the wrong place
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

	aiScene* scene = (aiScene*)importer.ReadFile(FilePath,
		aiProcess_Triangulate | aiProcess_CalcTangentSpace |
		aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords | aiProcess_FindInstances | aiProcess_ValidateDataStructure |
		aiProcess_OptimizeMeshes | 0);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		return SSceneRoot;
	}
	else
	{
		SSceneRoot.reset(generate_scene(scene));
	}
}

SScene::SakuraSceneNode* SSceneImporter::generate_scene(aiScene* scene, const uint32_t p_flags)
{
	SScene::SakuraSceneNode* newNodeRoot = new SScene::SakuraSceneNode();
	// Gen static Mesh Nodes
	if (scene->mRootNode)
	{
		for (auto i = 0u; i < scene->mRootNode->mNumChildren; i++)
		{
			generate_node(scene->mRootNode->mChildren[i], newNodeRoot);
		}
	}
	return newNodeRoot;
}

void SSceneImporter::generate_node(const aiNode* assimp_node, SScene::SakuraSceneNode* parentNode)
{
	SScene::SakuraSceneNode* newNode = nullptr;
	std::string node_name = assimp_node->mName.C_Str();
	//STransform node_transform = assimp_node->mTransformation;
}
