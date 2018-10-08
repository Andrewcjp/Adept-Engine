#include "MeshLoader.h"
#include "Rendering/Core/Mesh.h"
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include <include/assimp/postprocess.h>
#include "include/glm/gtx/transform.hpp"
#include "Core/EngineInc.h"
#include "Core/Platform/PlatformCore.h"
const glm::vec3 MeshLoader::DefaultScale = glm::vec3(1.0f, 1.0f, 1.0f);

bool MeshLoader::LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<int> &indices)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename.c_str(),
		aiProcess_Triangulate
		| aiProcess_CalcTangentSpace
		//| aiProcess_GenSmoothNormals
		//| aiProcess_FixInfacingNormals
	);

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
