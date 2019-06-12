#include "MeshLoader.h"
#include "Archive.h"
#include "Core/EngineInc.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Utils/DebugDrawers.h"
#include "Rendering/Core/Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
MeshLoader* MeshLoader::Instance = nullptr;

const glm::vec3 MeshLoader::DefaultScale = glm::vec3(1.0f, 1.0f, 1.0f);
void TraverseNodeTree(std::vector<aiNode*>& nodes, aiNode* currentnode)
{
	for (unsigned int i = 0; i < currentnode->mNumChildren; i++)
	{
		TraverseNodeTree(nodes, currentnode->mChildren[i]);
		nodes.push_back(currentnode->mChildren[i]);
	}
}

//#Anim: todo: this should be optimized; construct a sumed transfrom map?
bool FindMeshInNodeTree(std::vector<aiNode*> & nodes, const aiMesh* mesh, const aiScene* scene, aiMatrix4x4& transfrom, MeshLoader::FMeshLoadingSettings& Settings)
{
	if (nodes.size() == 0)
	{
		transfrom = scene->mRootNode->mTransformation;
		return true;
	}
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
///this loads only an animation from a file and adds it to a Skeletal mesh
///#Anim  validate bones are the same
bool MeshLoader::LoadAnimOnly(std::string filename, SkeletalMeshEntry * SkeletalMesh, std::string Name, FMeshLoadingSettings& Settings)
{
	Assimp::Importer* importer = new Assimp::Importer();

	unsigned int Flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights | aiProcess_SplitByBoneCount;
	if (Settings.GenerateIndexed)
	{
		Flags |= aiProcess_JoinIdenticalVertices;//aiProcess_MakeLeftHanded, aiProcess_OptimizeMeshes
	}
	const aiScene* scene = importer->ReadFile(filename.c_str(), Flags);
	if (scene == nullptr)
	{
		return false;
	}

	for (uint i = 0; i < scene->mNumAnimations; i++)
	{
		std::string Animname = Name;
		if (Animname.length() == 0)
		{
			Animname = scene->mAnimations[i]->mName.C_Str();
		}
		if (i > 0)
		{
			Animname += std::to_string(i);
		}
		Settings.AnimSettings.AssimpAnim = scene->mAnimations[i];
		SkeletalMesh->AnimNameMap.emplace(Animname, Settings.AnimSettings);
	}
	//#ANIM: memory leak here fix once moved to own anim system
	return true;
}

bool MeshLoader::LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<MeshEntity*> &Meshes, SkeletalMeshEntry** pSkeletalEntity)
{
	Assimp::Importer* importer = new Assimp::Importer();
	importer->SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
	unsigned int Flags = aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights /*aiProcess_OptimizeGraph*/ | aiProcess_SplitByBoneCount;
	if (Settings.GenerateIndexed)
	{
		Flags |= aiProcess_JoinIdenticalVertices;
	}
	if (Settings.FlipUVs)
	{
		Flags |= aiProcess_FlipUVs;
	}
	Flags |= aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
	const aiScene* scene = importer->ReadFile(filename.c_str(), Flags);

	if (!scene)
	{
		Log::OutS << "Mesh load failed!: " << filename << Log::OutS;
		return false;
	}
	const bool HasAnim = scene->HasAnimations();
	SkeletalMeshEntry* SKel = nullptr;
	if (HasAnim)
	{
		if (SKel == nullptr)
		{
			SKel = new SkeletalMeshEntry(scene->mAnimations[0]);
			*pSkeletalEntity = SKel;
			SKel->InitScene(scene);
		}
		for (uint i = 0; i < scene->mNumAnimations; i++)
		{
			SKel->AnimNameMap.emplace(std::string(/*scene->mAnimations[i]->mName.C_Str()*/"root"), AnimationClip(scene->mAnimations[i]));
		}
	}

	std::vector<OGLVertex> vertices;
	std::vector<IndType> indices;
	std::vector<aiNode*> NodeArray;
	TraverseNodeTree(NodeArray, scene->mRootNode);
	for (unsigned int modeli = 0; modeli < scene->mNumMeshes; modeli++)
	{
		const aiMesh* model = scene->mMeshes[modeli];
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
		//Hacky hack!
		glm::vec2 UVScale = Settings.UVScale;
		if (std::string(model->mName.C_Str()) == "Cube.001_Cube.002")
		{
			UVScale = glm::vec2(1);
		}
		//End Hacky hack
		for (unsigned int i = 0; i < model->mNumVertices; i++)
		{
			aiVector3D* pPos = &(model->mVertices[i]);
			const aiVector3D* pNormal = model->HasNormals() ? &(model->mNormals[i]) : &aiZeroVector;
			const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
			const aiVector3D* pTangent = model->HasTangentsAndBitangents() ? &(model->mTangents[i]) : &aiZeroVector;
			//	if (!scene->HasAnimations())
			{
				*pPos = transfrom * (*pPos);
			}

			OGLVertex vert(glm::vec3(pPos->x, pPos->y, pPos->z),
				glm::vec2(pTexCoord->x, pTexCoord->y)*UVScale,
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
			std::vector<VertexBoneData> Bones;
			Bones.resize(model->mNumVertices);
			SKel->LoadBones(modeli, model, Bones);
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
	if (!scene->HasAnimations())//#Anim extract Animations to Smaller and fast format
	{
		importer->FreeScene();
		SafeDelete(importer);
	}
	return true;
}

bool MeshLoader::LoadMeshFromFile_Direct(std::string filename, FMeshLoadingSettings& Settings, std::vector<OGLVertex> &vertices, std::vector<IndType>& indices)
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
	std::vector<aiNode*> NodeArray;
	TraverseNodeTree(NodeArray, scene->mRootNode);
	int basevert = 0;
	for (unsigned int modeli = 0; modeli < scene->mNumMeshes; modeli++)
	{
		const aiMesh* model = scene->mMeshes[modeli];//todo: temp!
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
			const aiVector3D* pNormal = &(model->mNormals[i]);
			const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
			const aiVector3D* pTangent = model->HasTangentsAndBitangents() ? &(model->mTangents[i]) : &aiZeroVector;

			*pPos = transfrom * (*pPos);

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
			indices.push_back(basevert + face.mIndices[0]);
			indices.push_back(basevert + face.mIndices[1]);
			indices.push_back(basevert + face.mIndices[2]);
		}
		basevert += model->mNumVertices;
	}
	importer.FreeScene();
	return true;
}



MeshLoader* MeshLoader::Get()
{
	if (Instance == nullptr)
	{
		Instance = new MeshLoader();
	}
	return Instance;
}

SkeletalMeshEntry::SkeletalMeshEntry(aiAnimation* anim)
{
	SetAnim(anim);
}

void SkeletalMeshEntry::SetAnim(const AnimationClip& anim)
{
	CurrentAnim = anim;
	if (anim.AssimpAnim->mTicksPerSecond != 0)
	{
		MaxTime = (float)anim.AssimpAnim->mDuration / (float)anim.AssimpAnim->mTicksPerSecond;
	}
	else
	{
		MaxTime = (float)anim.AssimpAnim->mDuration / 30.0f;
	}
}

glm::vec3 GetPos(glm::mat4 model)
{
	return glm::vec3(model[3][0], model[3][1], model[3][2]);
}

void SkeletalMeshEntry::RenderBones(Transform* T)
{
	for (int i = 0; i < FinalBoneTransforms.size(); i++)
	{
		const float C = 1.0f;
		//if (i == 29)
		{
			glm::mat4x4 Model = (T->GetModel());
			glm::vec3 LocalSpacePos = GetPos(FinalBoneTransforms[i]);
			LocalSpacePos = glm::vec4(LocalSpacePos, 1.0f) * glm::transpose(Model);
			DebugDrawers::DrawDebugSphere(LocalSpacePos, 0.2f, glm::vec3(C, 0, 0));
		}
	}
}

void SkeletalMeshEntry::Tick(float Delta)
{

	CurrnetTime += Delta * CurrentAnim.Rate;
	CurrnetTime = glm::clamp(CurrnetTime, 0.0f, MaxTime);
	if (CurrnetTime >= MaxTime)
	{
		CurrnetTime = 0.0f;
	}
	ReadNodes(CurrnetTime, Scene->mRootNode, glm::mat4(1), CurrentAnim.AssimpAnim);
	FinalBoneTransforms.resize(m_NumBones);
	for (int i = 0; i < m_NumBones; i++)
	{//Summing of transform is wonrG!
		FinalBoneTransforms[i] = m_BoneInfo[i].FinalTransformation;
	}

}

void SkeletalMeshEntry::PlayAnimation(std::string name)
{
	auto itor = AnimNameMap.find(name);
	if (itor != AnimNameMap.end())
	{
		SetAnim(itor->second);
	}
	else
	{
		Log::LogMessage("Failed find animation " + name);
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

void SkeletalMeshEntry::LoadBones(uint MeshIndex, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones)
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
			const uint VertexID = pMesh->mBones[i]->mWeights[j].mVertexId;
			const float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
			if (Weight >= 0.1f)
			{
				Bones[VertexID].AddBoneData(BoneIndex, Weight);
			}
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
	//assert(0);
	return 0;
}

void SkeletalMeshEntry::Release()
{
	MemoryUtils::DeleteVector(MeshEntities);
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
	return 0;
}
#define NO_INTERP 0

void SkeletalMeshEntry::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1)
	{
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
#if NO_INTERP
	Out = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	return;
#endif
	uint NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	Factor = glm::clamp(Factor, 0.0f, 1.0f);
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
#if NO_INTERP
	Out = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	return;
#endif
	uint NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	//	assert(Factor >= 0.0f && Factor <= 1.0f);
	Factor = glm::clamp(Factor, 0.0f, 1.0f);
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
#if NO_INTERP
	Out = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	return;
#endif

	uint NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	//assert(Factor >= 0.0f && Factor <= 1.0f);
	Factor = glm::clamp(Factor, 0.0f, 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

void SkeletalMeshEntry::ReadNodes(float time, const aiNode* pNode, const glm::mat4 ParentTransfrom, const aiAnimation* Anim)
{
	float Animtime = time * (float)Anim->mTicksPerSecond;
	std::string NodeName(pNode->mName.data);
	glm::mat4 NodeTransformation = ToGLM(pNode->mTransformation);
	const aiNodeAnim* pNodeAnim = FindNodeAnim(Anim, NodeName);
	if (pNodeAnim)
	{
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, Animtime, pNodeAnim);
		glm::mat4 ScalingM = glm::scale(ToGLM(Scaling));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, Animtime, pNodeAnim);
		glm::mat4 RotationM = glm::mat4(glm::normalize(glm::quat(ToGLM(RotationQ.GetMatrix()))));

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, Animtime, pNodeAnim);
		glm::mat4x4 TranslationM = glm::translate(ToGLM(Translation));

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;

		//NodeTransformation = glm::transpose(NodeTransformation);
		//NodeTransformation = glm::mat4(1);
	}

	glm::mat4x4 ParentComp = glm::mat4(1);
#if 0
	const aiNode* CurrentNode = pNode;
	std::vector<glm::mat4>  Transforms;
	while (CurrentNode != Scene->mRootNode)
	{
		ParentComp *= ToGLM(CurrentNode->mTransformation);
		Transforms.push_back(ToGLM(CurrentNode->mTransformation));
		CurrentNode = CurrentNode->mParent;
	}
	for (int i = 0; i < Transforms.size(); i++)
	{
		ParentComp *= Transforms[i];
	}
#endif
	if (pNode->mParent != nullptr)
	{
		//ParentComp = ToGLM(pNode->mParent->mTransformation);
		ParentComp = ParentTransfrom;
	}
	glm::mat4 GlobalTransformation = (ParentComp)* ToGLM(pNode->mTransformation)*NodeTransformation;
	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end())
	{
		uint BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = ModelInvTransfrom * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (uint i = 0; i < pNode->mNumChildren; i++)
	{
		ReadNodes(time, pNode->mChildren[i], GlobalTransformation, Anim);
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
	Log::LogMessage("NUM_BONES_PER_VEREX exceeded");
	int Lowest = 0;
	float value = 2.0f;
	for (uint i = 0; i < NUM_BONES_PER_VEREX; i++)
	{
		if (Weights[i] < value)
		{
			Lowest = i;
			value = Weights[i];
		}
	}
	IDs[Lowest] = BoneID;
	Weights[Lowest] = glm::clamp(Weight, 0.0f, 1.0f);

	// should never get here - more bones than we have space for
	//assert(0);
}

void MeshLoader::RegisterLoad(std::string path, Mesh * mesh)
{
	Get()->CreatedMeshes.emplace(path, mesh);
}

void MeshLoader::ShutDown()
{
	Instance->DestoryMeshes();
	SafeDelete(Instance);
}

void MeshLoader::DestoryMeshes()
{
	//std::map<std::string, Mesh*>::iterator it;
	//for (it = CreatedMeshes.begin(); it != CreatedMeshes.end(); it++)
	//{
	//	EnqueueSafeRHIRelease(it->second);
	//}
}

Mesh * MeshLoader::TryLoadFromCache(std::string Path)
{
	auto Itor = CreatedMeshes.find(Path);
	if (Itor != CreatedMeshes.end())
	{
		if (Itor->second->GetSkeletalMesh() != nullptr)//execute Copy
		{
			return nullptr;
		}
		Mesh* NewInstance = new Mesh();
		NewInstance->InstanceFrom(Itor->second);
		return NewInstance;
	}
	return nullptr;
}