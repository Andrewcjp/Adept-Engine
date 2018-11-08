#include "MeshLoader.h"
#include "Rendering/Core/Mesh.h"
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include <include/assimp/postprocess.h>
#include "Core/EngineInc.h"
#include "Core/Platform/PlatformCore.h"
const glm::vec3 MeshLoader::DefaultScale = glm::vec3(1.0f, 1.0f, 1.0f);
void TraverseNodeTree(std::vector<aiNode*>& nodes, aiNode* currentnode)
{
	for (unsigned int i = 0; i < currentnode->mNumChildren; i++)
	{
		TraverseNodeTree(nodes, currentnode->mChildren[i]);
		nodes.push_back(currentnode->mChildren[i]);
	}
}

//todo: this should be optimized; construct a sumed transfrom map?
bool FindMeshInNodeTree(std::vector<aiNode*> & nodes, const aiMesh* mesh, const aiScene* scene, aiMatrix4x4& transfrom)
{
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		for (unsigned int j = 0; j < nodes[i]->mNumMeshes; j++)
		{
			if (scene->mMeshes[nodes[i]->mMeshes[j]] == mesh)
			{
				aiNode* curerntnode = nodes[i];
				while (curerntnode != scene->mRootNode)
				{
					transfrom *= curerntnode->mTransformation;
					curerntnode = curerntnode->mParent;
				}
				return true;
			}
		}
	}
	return false;
}

bool MeshLoader::LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<MeshEntity*> &Meshes)
{
	Assimp::Importer importer;
	unsigned int Flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
	if (Settings.GenerateIndexed)
	{
		Flags |= aiProcess_JoinIdenticalVertices;
	}
	if (Settings.FlipUVs)
	{
		Flags |= aiProcess_FlipUVs;
	}
	const aiScene* scene = importer.ReadFile(filename.c_str(), Flags);

	if (!scene)
	{
		Log::OutS << "Mesh load failed!: " << filename << Log::OutS;
		return false;
	}
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	std::vector<aiNode*> NodeArray;
	TraverseNodeTree(NodeArray, scene->mRootNode);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* model = scene->mMeshes[i];
		const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
		aiMatrix4x4 transfrom;
		const bool ValidTransfrom = FindMeshInNodeTree(NodeArray, model, scene, transfrom);
		DebugEnsure(ValidTransfrom);
		for (unsigned int i = 0; i < model->mNumVertices; i++)
		{
			aiVector3D* pPos = &(model->mVertices[i]);
			const aiVector3D* pNormal = &(model->mNormals[i]);
			const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
			const aiVector3D* pTangent = model->HasTangentsAndBitangents() ? &(model->mTangents[i]) : &aiZeroVector;

			*pPos = transfrom * (*pPos);

			OGLVertex vert(glm::vec3(pPos->x, pPos->y, pPos->z),
				glm::vec2(pTexCoord->x, pTexCoord->y)*Settings.UVScale,
				glm::vec3(pNormal->x, pNormal->y, pNormal->z),
				glm::vec3(pTangent->x, pTangent->y, pTangent->z));

			vertices.push_back(vert);
		}

		if (Settings.Scale != DefaultScale)
		{
			for (int i = 0; i < vertices.size(); i++)
			{
				glm::vec4 Pos = glm::vec4(vertices[i].m_position.xyz, 1.0f);
				Pos = Pos * glm::scale(Settings.Scale);
				vertices[i].m_position = Pos.xyz;
			}
		}
		for (unsigned int i = 0; i < model->mNumFaces; i++)
		{
			const aiFace& face = model->mFaces[i];
			ensure(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
		MeshEntity* newmesh = new MeshEntity(Settings, vertices, indices);
		//workaround for weird extra material from blender models
		if (scene->mNumMaterials > 1)
		{
			newmesh->MaterialIndex = model->mMaterialIndex - 1;
		}
		else
		{
			newmesh->MaterialIndex = model->mMaterialIndex;
		}
		Meshes.push_back(newmesh);

		vertices.clear();
		indices.clear();
	}
	importer.FreeScene();
	return true;
}

bool MeshLoader::LoadMeshFromFile_Direct(std::string filename, FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<int>& indices)
{
	Assimp::Importer importer;
	unsigned int Flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
	if (Settings.GenerateIndexed)
	{
		Flags |= aiProcess_JoinIdenticalVertices;
	}
	if (Settings.FlipUVs)
	{
		Flags |= aiProcess_FlipUVs;
	}
	const aiScene* scene = importer.ReadFile(filename.c_str(), Flags);

	if (!scene)
	{
		Log::OutS << "Mesh load failed!: " << filename << Log::OutS;
		return false;
	}
	const aiMesh* model = scene->mMeshes[0];
	const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
	for (unsigned int i = 0; i < model->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(model->mVertices[i]);
		const aiVector3D* pNormal = &(model->mNormals[i]);
		const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
		const aiVector3D* pTangent = model->HasTangentsAndBitangents() ? &(model->mTangents[i]) : &aiZeroVector;

		OGLVertex vert(glm::vec3(pPos->x, pPos->y, pPos->z),
			glm::vec2(pTexCoord->x, pTexCoord->y),
			glm::vec3(pNormal->x, pNormal->y, pNormal->z),
			glm::vec3(pTangent->x, pTangent->y, pTangent->z));

		vertices.push_back(vert);
	}
	if (Settings.Scale != DefaultScale)
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			glm::vec4 Pos = glm::vec4(vertices[i].m_position.xyz, 1.0f);
			Pos = Pos * glm::scale(Settings.Scale);
			vertices[i].m_position = Pos.xyz;
		}
	}
	for (unsigned int i = 0; i < model->mNumFaces; i++)
	{
		const aiFace& face = model->mFaces[i];
		ensure(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	importer.FreeScene();
	return true;
}