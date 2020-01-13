#include "MeshImporter.h"
#include "../../GraphicsCommon/UploadVertices.h"
#include <iostream>
#include "Common/HikaUtils/HikaCommonUtils/HikaCommonUtil.h"
#include <DirectXMath.h>
#include "../../../Core/Nodes/SakuraSceneNode.hpp"
using namespace std;
using namespace HikaD3DUtils;
using namespace tinyply;

std::unique_ptr<Dx12MeshGeometry> MeshImporter::ImportMesh(ID3D12Device* device, ID3D12GraphicsCommandList* CommandList, std::string FilePath, ESupportFileForm FileForm /*= ESupportFileForm::TEXT*/)
{
	auto geo = std::make_unique<Dx12MeshGeometry>();
	std::vector<StandardVertex> vertices;
	std::vector<std::uint32_t> indices;
	UINT vcount = 0;
	UINT tcount = 0;
	if (FileForm == ESupportFileForm::TEXT)
	{
		ifstream fin(FilePath);

		if (!fin)
		{
			// Failed to construct fin from FilePath
			std::string ErrorMsg = std::string("MeshImporter Error!\nifstream error: ") + FilePath + std::string(" failed to construct ifstream target. ");
			LPCWSTR LPCWSTRError = HikaCommonUtils::stringToLPCWSTR(ErrorMsg);
			MessageBox(0, LPCWSTRError, 0, 0);
			return geo;
		}

		std::string ignore;

		fin >> ignore >> vcount;
		fin >> ignore >> tcount;
		fin >> ignore >> ignore >> ignore >> ignore;

		vertices.resize(vcount);
		for (UINT i = 0; i < vcount; ++i)
		{
			fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
			fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
			vertices[i].Color = { .27f, 0.28f, .25f, 1.f };
		}

		fin >> ignore;
		fin >> ignore;
		fin >> ignore; // } TriangleList { // Index List Start

		indices.resize(3 * tcount);
		for (UINT i = 0; i < tcount; ++i)
		{
			fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
		}

		// Finish ifstream load
		fin.close();
	}
	else if (FileForm == ESupportFileForm::ASSIMP_SUPPORTFILE)
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
		// Cannot remove pivot points because the static mesh will be in the wrong place
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

		const aiScene* scene = importer.ReadFile(FilePath,
			aiProcess_Triangulate | aiProcess_CalcTangentSpace | 
			 aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords|
			aiProcess_TransformUVCoords | aiProcess_FindInstances | aiProcess_ValidateDataStructure|
			aiProcess_OptimizeMeshes | 0);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			return ImportMesh(device, CommandList, "Models/skull.txt");
		}
		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene, vertices, indices);
	}
	else if (FileForm == ESupportFileForm::PLY)
	{
		read_ply_file(FilePath, vertices, indices);
	}
	else if (FileForm == ESupportFileForm::COUNT)
	{
		assert(0 || "Mesh importer assert: Unexpected file form, do not use ESupportFileForm as input. ");
		return geo;
	}
		

	// Pack the indices of all the meshes into on index buffer
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(StandardVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);

	// Pack the indices and vertices of all the meshes into buffer
	{
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(StandardVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint32_t);


		geo->Name = FilePath;

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device, CommandList,
			vertices.data(), vbByteSize, geo->VertexBufferUploader);
		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device, CommandList,
			indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(StandardVertex);
		geo->VertexBufferByteSize = vbByteSize;
		geo->IndexBufferByteSize = ibByteSize;
		geo->IndexFormat = DXGI_FORMAT_R32_UINT;

		SubmeshGeometry submesh;
		// Only one submesh
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		geo->DrawArgs["mesh"] = submesh;
	}
	
	return geo;
}

void HikaD3DUtils::MeshImporter::processNode(aiNode* node, const aiScene* scene, 
		std::vector<StandardVertex>& vertices, std::vector<std::uint32_t>& indices)
{
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene, vertices, indices);
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, vertices, indices);
	}
}

void HikaD3DUtils::MeshImporter::processMesh(aiMesh* mesh, const aiScene* scene, 
		std::vector<StandardVertex>& vertices, std::vector<std::uint32_t>& indices)
{
	// Walk through each of the mesh's vertices
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

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

}

void HikaD3DUtils::MeshImporter::read_ply_file(const std::string& filepath, std::vector<StandardVertex>& InVertices, std::vector<std::uint32_t>& InIndices)
{
	try
	{
		std::ifstream ss(filepath, std::ios::binary);
		if (ss.fail()) throw std::runtime_error("failed to open " + filepath);

		PlyFile file;
		file.parse_header(ss);

		std::cout << "........................................................................\n";
		for (auto c : file.get_comments()) std::cout << "Comment: " << c << std::endl;
		for (auto e : file.get_elements())
		{
			std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
			for (auto p : e.properties) std::cout << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
		}
		std::cout << "........................................................................\n";

		// Tinyply treats parsed data as untyped byte buffers. See below for examples.
		std::shared_ptr<PlyData> vertices, normals, faces, texcoords;

		// The header information can be used to programmatically extract properties on elements
		// known to exist in the header prior to reading the data. For brevity of this sample, properties 
		// like vertex position are hard-coded: 
		try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		// Providing a list size hint (the last argument) is a 2x performance improvement. If you have 
		// arbitrary ply files, it is best to leave this 0. 
		try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		manual_timer read_timer;

		read_timer.start();
		file.read(ss);
		read_timer.stop();

		std::cout << "Reading took " << read_timer.get() / 1000.f << " seconds." << std::endl;
		if (vertices) std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
		if (normals) std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
		if (texcoords) std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
		if (faces) std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;

		// type casting to your own native types - Option B
		{
			UINT InVertexBaseCount = InVertices.size();
			UINT InIndexBaseCount = InIndices.size();

			std::vector<float3> verts_floats(vertices->count);
			InVertices.resize(InVertexBaseCount + vertices->count);


			std::vector<int32_t> indices_uint32(faces->count * 3);
			InIndices.resize(InIndexBaseCount + faces->count * 3);

			std::vector<float3> norms_floats(normals ? normals->count : 1);
			
			std::vector<float2> texcoords_floats(texcoords ? texcoords->count : 1);


			if (vertices->t == tinyply::Type::FLOAT32) {
				const size_t numVerticesBytes = vertices->buffer.size_bytes();
				const size_t numNormBytes = normals ? normals->buffer.size_bytes() : 0;
				const size_t numIndicesBytes = faces->buffer.size_bytes();
				std::memcpy(verts_floats.data(), vertices->buffer.get(), numVerticesBytes);
				std::memcpy(indices_uint32.data(), faces->buffer.get(), numIndicesBytes);
				if(normals) std::memcpy(norms_floats.data(), normals->buffer.get(), numNormBytes);
				for (UINT i = 0; i < vertices->count; ++i)
				{
					InVertices[InVertexBaseCount + i].Pos.x = verts_floats[i].x;
					InVertices[InVertexBaseCount + i].Pos.y = verts_floats[i].y;
					InVertices[InVertexBaseCount + i].Pos.z = verts_floats[i].z;
					if (normals)
					{
						InVertices[InVertexBaseCount + i].Normal.x = norms_floats[i].x;
						InVertices[InVertexBaseCount + i].Normal.y = norms_floats[i].y;
						InVertices[InVertexBaseCount + i].Normal.z = norms_floats[i].z;
					}
					if (texcoords)
					{
						InVertices[InVertexBaseCount + i].TexC.x = texcoords_floats[i].x;
						InVertices[InVertexBaseCount + i].TexC.y = texcoords_floats[i].y;
					}
				}

				if (!normals)
				{
					// face normal
					for (int i = 0; i < InVertices.size()-3; i += 3)
					{
						DirectX::XMFLOAT3 ab =
						{
							InVertices[i + 1].Pos.x - InVertices[i].Pos.x,
							InVertices[i + 1].Pos.y - InVertices[i].Pos.y,
							InVertices[i + 1].Pos.z - InVertices[i].Pos.z
						};
						DirectX::XMFLOAT3 bc =
						{
							-InVertices[i + 2].Pos.x + InVertices[i + 1].Pos.x,
							-InVertices[i + 2].Pos.y + InVertices[i + 1].Pos.y,
							-InVertices[i + 2].Pos.z + InVertices[i + 1].Pos.z
						};
						DirectX::XMFLOAT3 normal;
						DirectX::XMVECTOR abvec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&ab));
						DirectX::XMVECTOR bcvec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&bc));
						DirectX::XMVECTOR normalvec = DirectX::XMVector3Cross(abvec, bcvec);
						XMStoreFloat3(&normal, normalvec);
						InVertices[i].Normal.x = normal.x;
						InVertices[i].Normal.y = normal.y;
						InVertices[i].Normal.z = normal.z;
						InVertices[i + 1].Normal.x = normal.x;
						InVertices[i + 1].Normal.y = normal.y;
						InVertices[i + 1].Normal.z = normal.z;
						InVertices[i + 2].Normal.x = normal.x;
						InVertices[i + 2].Normal.y = normal.y;
						InVertices[i + 2].Normal.z = normal.z;
					}
					for (int i = 0; i < InVertices.size(); i++)
					{
						DirectX::XMVECTOR normvec = DirectX::XMVector3Normalize(XMLoadFloat3(&InVertices[i].Normal));
						XMStoreFloat3(&InVertices[i].Normal, normvec);
					}
				}
				for (UINT i = 0; i < faces->count; i++)
				{
					InIndices[InIndexBaseCount + 0 + i * 3] = indices_uint32[0 + i * 3];
					InIndices[InIndexBaseCount + 1 + i * 3] = indices_uint32[1 + i * 3];
					InIndices[InIndexBaseCount + 2 + i * 3] = indices_uint32[2 + i * 3];
				}
			}
			if (vertices->t == tinyply::Type::FLOAT64) {
				assert(0);
				std::cout << "WTF64";
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
	}
}

