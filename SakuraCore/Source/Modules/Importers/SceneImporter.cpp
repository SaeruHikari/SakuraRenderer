#include "SceneImporter.h"
#include <Framework\Core\Nodes\SakuraSceneNode.hpp>
#include <Framework\Core\Nodes\EngineNodes\SStaticMeshNode.hpp>
#include <Framework\GraphicTypes\GraphicsCommon\UploadVertices.h>

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
	return SSceneRoot;
}

SScene::SakuraSceneNode* SSceneImporter::generate_scene(aiScene* scene, const uint32_t p_flags)
{
	SScene::SakuraSceneNode* newNodeRoot = new SScene::SakuraSceneNode();
	newNodeRoot->SetNodeName("ImportedScene");
	// Gen static Mesh Nodes
	if (scene->mRootNode)
	{
		for (auto i = 0u; i < scene->mRootNode->mNumChildren; i++)
		{
			generate_node(scene->mRootNode->mChildren[i], scene, newNodeRoot);
		}
	}
	return newNodeRoot;
}

void SSceneImporter::generate_node(const aiNode* assimp_node, const aiScene* scene, SScene::SakuraSceneNode* parentNode)
{
	SScene::SakuraSceneNode* newNode = nullptr;
	std::string node_name = assimp_node->mName.C_Str();
	STransform node_transform;
	AssimpUtils::LoadTransformFromAssimpMatrix(node_transform, assimp_node->mTransformation);
	// Import mesh
	if (assimp_node->mNumMeshes > 0)
	{
		std::vector<StandardVertex> vertices;
		std::vector<std::uint32_t> indices;
		std::vector<std::pair<SubmeshDesc, SubmeshGeometry>> subMeshDatas;
		subMeshDatas.resize(assimp_node->mNumMeshes);
		newNode = new SStaticMeshNode(node_transform.Location, node_name);
		// Deal with piece of submesh
		for (size_t i = 0; i < assimp_node->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[assimp_node->mMeshes[i]];
			
			subMeshDatas[i].first.Name = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
			subMeshDatas[i].first.Mat = "DefaultMat";

			subMeshDatas[i].second.BaseVertexLocation = vertices.size();
			subMeshDatas[i].second.StartIndexLocation = indices.size();
			
			processMesh(mesh, scene, vertices, indices);

			subMeshDatas[i].second.IndexCount = indices.size() - subMeshDatas[i].second.StartIndexLocation;
			subMeshDatas[i].second.VertexCount = vertices.size() - subMeshDatas[i].second.BaseVertexLocation;
		}
		((SStaticMeshNode*)newNode)->SetMesh(vertices, indices, node_name, subMeshDatas);
	}
	else
	{
		newNode = new SScene::SakuraSceneNode();
	}
	parentNode->Attach(newNode);
	for (auto i = 0u; i < assimp_node->mNumChildren; i++)
	{
		generate_node(assimp_node->mChildren[i], scene, newNode);
	}
}

void SSceneImporter::processMesh(aiMesh* mesh, const aiScene* scene,
	std::vector<StandardVertex>& vertices, std::vector<std::uint32_t>& indices)
{
	// Walk through each of the mesh's vertices
	vertices.reserve(vertices.size() + mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		StandardVertex newVertex;
		newVertex.Pos.x = mesh->mVertices[i].x;
		newVertex.Pos.y = mesh->mVertices[i].y;
		newVertex.Pos.z = mesh->mVertices[i].z;

		newVertex.Normal.x = mesh->mNormals[i].x;
		newVertex.Normal.y = mesh->mNormals[i].y;
		newVertex.Normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			newVertex.TexC.x = mesh->mTextureCoords[0][i].x;
			newVertex.TexC.y = mesh->mTextureCoords[0][i].y;
		}
		else newVertex.TexC = { 0.f, 0.f };

		// Tangent
		newVertex.Tangent.x = mesh->mTangents[i].x;
		newVertex.Tangent.y = mesh->mTangents[i].y;
		newVertex.Tangent.z = mesh->mTangents[i].z;
		// Bit tangent
		//...
		vertices.push_back(newVertex);
	}
	indices.reserve(indices.size() + mesh->mNumFaces * 3);
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}
