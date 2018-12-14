#include "MeshLoader.h"
#include "Archive.h"
#include "Core/EngineInc.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/Core/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Core/Utils/VectorUtils.h"
#include "assimp/anim.h"


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
bool FindMeshInNodeTree(std::vector<aiNode*> & nodes, const aiMesh* mesh, const aiScene* scene, aiMatrix4x4& transfrom, MeshLoader::FMeshLoadingSettings& Settings)
{
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		for (unsigned int j = 0; j < nodes[i]->mNumMeshes; j++)
		{
			if (scene->mMeshes[nodes[i]->mMeshes[j]] == mesh)
			{
				aiNode* curerntnode = nodes[i];
				if (VectorUtils::Contains(Settings.IgnoredMeshObjectNames, std::string(curerntnode->mName.C_Str())))
				{
					return false;
				}
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

void MeshLoader::FMeshLoadingSettings::Serialize(Archive * A)
{
	ArchiveProp(Scale);
	ArchiveProp(UVScale);
	ArchiveProp(InitOnAllDevices);
	ArchiveProp(CreatePhysxMesh);
	ArchiveProp(GenerateIndexed);
	ArchiveProp(FlipUVs);
}

bool MeshLoader::LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<MeshEntity*> &Meshes, SkeletalMeshEntry** pSkeletalEntity)
{
	Assimp::Importer* importer = new Assimp::Importer();
	unsigned int Flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace;
	if (Settings.GenerateIndexed)
	{
		Flags |= aiProcess_JoinIdenticalVertices;
	}
	if (Settings.FlipUVs)
	{
		Flags |= aiProcess_FlipUVs;
	}
	const aiScene* scene = importer->ReadFile(filename.c_str(), Flags);

	if (!scene)
	{
		Log::OutS << "Mesh load failed!: " << filename << Log::OutS;
		return false;
	}
	const bool HasAnim = scene->HasAnimations();
	if (scene->HasAnimations())
	{
		float t = 0;
	}
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	std::vector<aiNode*> NodeArray;
	SkeletalMeshEntry* SKel = nullptr;
	TraverseNodeTree(NodeArray, scene->mRootNode);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* model = scene->mMeshes[i];
		if (VectorUtils::Contains(Settings.IgnoredMeshObjectNames, std::string(model->mName.C_Str())))
		{
			continue;
		}
		const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
		aiMatrix4x4 transfrom;
		const bool ValidTransfrom = FindMeshInNodeTree(NodeArray, model, scene, transfrom, Settings);
		if (!ValidTransfrom)
		{
			continue;
		}
		for (unsigned int i = 0; i < model->mNumVertices; i++)
		{
			aiVector3D* pPos = &(model->mVertices[i]);
			const aiVector3D* pNormal = model->HasNormals() ? &(model->mNormals[i]) : &aiZeroVector;
			const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
			const aiVector3D* pTangent = model->HasTangentsAndBitangents() ? &(model->mTangents[i]) : &aiZeroVector;
			if (!scene->HasAnimations())
			{
				*pPos = transfrom * (*pPos);
			}

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
		MeshEntity* newmesh = nullptr;
		if (HasAnim)
		{
			if (SKel == nullptr)
			{
				SKel = new SkeletalMeshEntry(scene->mAnimations[0]);
				*pSkeletalEntity = SKel;
				SKel->InitScene(scene);
			}
			std::vector<VertexBoneData> Bones;
			Bones.resize(model->mNumVertices);
			int BaseVertex = 0;
			SKel->LoadBones(i, model, Bones, BaseVertex);
			for (int i = 0; i < vertices.size(); i++)
			{
				vertices[i].m_boneIDs = glm::ivec4(Bones[i].IDs[0], Bones[i].IDs[1], Bones[i].IDs[2], Bones[i].IDs[3]);
				vertices[i].m_weights = glm::vec4(Bones[i].Weights[0], Bones[i].Weights[1], Bones[i].Weights[2], Bones[i].Weights[3]);
			}
			newmesh = new MeshEntity(Settings, vertices, indices);
			SKel->MeshEntities.push_back(newmesh);
		}
		else
		{
			newmesh = new MeshEntity(Settings, vertices, indices);
			Meshes.push_back(newmesh);
		}

		//workaround for weird extra material from blender models
		if (scene->mNumMaterials > 1)
		{
			newmesh->MaterialIndex = model->mMaterialIndex - 1;
		}
		else
		{
			newmesh->MaterialIndex = model->mMaterialIndex;
		}

		vertices.clear();
		indices.clear();
	}
	if (!scene->HasAnimations())//todo: extract Animations to Smaller and fast format
	{
		importer->FreeScene();
		SafeDelete(importer);
	}
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

SkeletalMeshEntry::SkeletalMeshEntry(aiAnimation* anim)
{
	if (anim->mTicksPerSecond != 0)
	{
		MaxTime = (float)anim->mDuration / (float)anim->mTicksPerSecond;
	}
	else
	{
		MaxTime = (float)anim->mDuration / 30.0f;
	}
}

void SkeletalMeshEntry::Tick(float Delta)
{
	CurrnetTime += Delta;
	CurrnetTime = glm::clamp(CurrnetTime, 0.0f, MaxTime);
	if (CurrnetTime == MaxTime)
	{
		CurrnetTime = 0.0f;
	}

	ReadNodes(CurrnetTime, Scene->mRootNode, glm::mat4(1));

	FinalBoneTransforms.resize(m_NumBones);
	for (int i = 0; i < m_NumBones; i++)
	{
		FinalBoneTransforms[i] = m_BoneInfo[i].FinalTransformation;
	}
}

glm::mat3x3 ToGLM(aiMatrix3x3& mat)
{
	return glm::rowMajor3(glm::mat3(mat.a1, mat.a2, mat.a3, mat.b1, mat.b2, mat.b3, mat.c1, mat.c2, mat.c3));
}

glm::mat4 ToGLM(aiMatrix4x4& mat)
{
	return glm::rowMajor4(glm::mat4(mat.a1, mat.a2, mat.a3, mat.a4, mat.b1, mat.b2, mat.b3, mat.b4, mat.c1, mat.c2, mat.c3, mat.c4, mat.d1, mat.d2, mat.d3, mat.d4));//assimp uses column major mats 
	//we use row major ones
}

glm::mat4 ToGLM(const aiMatrix4x4& mat)
{
	return glm::rowMajor4(glm::mat4(mat.a1, mat.a2, mat.a3, mat.a4, mat.b1, mat.b2, mat.b3, mat.b4, mat.c1, mat.c2, mat.c3, mat.c4, mat.d1, mat.d2, mat.d3, mat.d4));
}

glm::vec3 ToGLM(aiVector3D& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

void SkeletalMeshEntry::LoadBones(uint MeshIndex, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones, int BaseVertex)
{
	for (uint i = 0; i < pMesh->mNumBones; i++)
	{
		uint BoneIndex = 0;
		std::string BoneName(pMesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end())
		{
			// Allocate an index for a new bone
			BoneIndex = m_NumBones;
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneInfo[BoneIndex].BoneOffset = ToGLM(pMesh->mBones[i]->mOffsetMatrix);
			m_BoneMapping[BoneName] = BoneIndex;
		}
		else
		{
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (uint j = 0; j < pMesh->mBones[i]->mNumWeights; j++)
		{
			uint VertexID =/* MeshEntities[MeshIndex]->BaseVertex */BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			Bones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}
}

const aiNodeAnim* SkeletalMeshEntry::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (uint i = 0; i < pAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName)
		{
			return pNodeAnim;
		}
	}

	return NULL;
}



void SkeletalMeshEntry::InitScene(const aiScene* sc)
{
	Scene = sc;
	ModelInvTransfrom = ToGLM(sc->mRootNode->mTransformation);
	ModelInvTransfrom = glm::inverse(ModelInvTransfrom);
}

uint SkeletalMeshEntry::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (uint i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
		{
			return i;
		}
	}
	assert(0);
	return 0;
}

void SkeletalMeshEntry::Release()
{
	MemoryUtils::DeleteReleaseableVector(MeshEntities);

}


uint SkeletalMeshEntry::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);
	for (uint i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
		{
			return i;
		}
	}
	assert(0);
	return 0;
}

uint SkeletalMeshEntry::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (uint i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
		{
			return i;
		}
	}

	assert(0);

	return 0;
}


void SkeletalMeshEntry::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1)
	{
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	uint NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}
void SkeletalMeshEntry::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1)
	{
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	uint PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	uint NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}
void SkeletalMeshEntry::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1)
	{
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	uint NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void SkeletalMeshEntry::ReadNodes(float time, const aiNode * pNode, const glm::mat4 ParentTransfrom)
{
	std::string NodeName(pNode->mName.data);

	const aiAnimation* Anim = Scene->mAnimations[0];//todo: multi anims

	glm::mat4 NodeTransformation = ToGLM(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(Anim, NodeName);
	if (pNodeAnim)
	{
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, time, pNodeAnim);
		glm::mat4 ScalingM = glm::scale(ToGLM(Scaling));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, time, pNodeAnim);
		glm::mat4 RotationM = glm::mat4(ToGLM(RotationQ.GetMatrix()));

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, time, pNodeAnim);
		glm::mat4x4 TranslationM = glm::translate(ToGLM(Translation));

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;

	}
	glm::mat4 GlobalTransformation = ParentTransfrom * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end())
	{
		uint BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = ModelInvTransfrom * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (uint i = 0; i < pNode->mNumChildren; i++)
	{
		ReadNodes(time, pNode->mChildren[i], GlobalTransformation);
	}
}

void VertexBoneData::AddBoneData(uint BoneID, float Weight)
{
	for (uint i = 0; i < NUM_BONES_PER_VEREX; i++)
	{
		if (Weights[i] == 0.0)
		{
			IDs[i] = BoneID;
			Weights[i] = glm::clamp(Weight, 0.0f, 1.0f);
			return;
		}
	}
	// should never get here - more bones than we have space for
	assert(0);
}