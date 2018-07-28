#include "MeshLoader.h"
#include "Rendering/Core/Mesh.h"
#include "include/assimp/Importer.hpp"
#include "include/assimp/scene.h"
#include <include/assimp/postprocess.h>
#include "include/glm/gtx/transform.hpp"
const glm::vec3 MeshLoader::DefaultScale = glm::vec3(1.0f, 1.0f, 1.0f);

bool MeshLoader::LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<int> &indices)
{
	//m_meshData = 0;


	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename.c_str(),
		aiProcess_Triangulate
		| aiProcess_CalcTangentSpace
	//	| aiProcess_GenNormals
		| aiProcess_GenSmoothNormals
		
#if 1
		| aiProcess_FixInfacingNormals
#endif

	);

	if (!scene)
	{
		Log::OutS  << "Mesh load failed!: " << filename << Log::OutS;		
	}

	const aiMesh* model = scene->mMeshes[0];

	const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
	for (unsigned int i = 0; i < model->mNumVertices; i++)
	{
		const aiVector3D* pPos = &(model->mVertices[i]);
		const aiVector3D* pNormal = &(model->mNormals[i]);
		const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
		const aiVector3D* pTangent = &(model->mTangents[i]);

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
			glm::vec4 Pos = glm::vec4 (vertices[i].m_position.xyz,1.0f);
			Pos = Pos * glm::scale(Settings.Scale);
			vertices[i].m_position = Pos.xyz;
		}
	}
	for (unsigned int i = 0; i < model->mNumFaces; i++)
	{
		const aiFace& face = model->mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
	//InitMesh(&vertices[0], vertices.size(), (int*)&indices[0], indices.size(), false);
	///*Mesh* newmesh = new Mesh(&vertices[0], vertices.size(), (int*)&indices[0], indices.size())*/;

	return true;
}
