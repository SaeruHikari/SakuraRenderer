/*****************************************************************************************
							 Copyrights   SaeruHikari
CreateDate:							2020.1.1
Description:	   Import Scene from SSNF or Assimp support files.
Details:
*******************************************************************************************/
#pragma once
#include "../Assimp/AssimpDefinations.hpp"

namespace SScene
{
	class SakuraSceneNode;
}

struct StandardVertex;

static class SSceneImporter
{
public:
	static std::unique_ptr<SScene::SakuraSceneNode> ImportScene(std::string FilePath);
private:
	static SScene::SakuraSceneNode* generate_scene(aiScene* scene, const uint32_t p_flags = 0);
	static void generate_node(const aiNode* assimp_node, const aiScene* scene, SScene::SakuraSceneNode* parentNode);
	static void processMesh(aiMesh* mesh, const aiScene* scene,
		std::vector<StandardVertex>& vertices, std::vector<std::uint32_t>& indices);
};
