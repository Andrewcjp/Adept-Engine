#pragma once
#include "Rendering/Core/RenderBaseTypes.h"
#ifdef BUILD_ASSIMP
#include "assimp/vector3.h"
#include "assimp/quaternion.h"
#endif
struct MeshEntity;
struct aiScene;
class Archive;
struct SkeletalMeshEntry;
struct aiMesh;
struct aiAnimation;
struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiAnimation;
class Mesh;
class Transform;

///Represents one animation clip and all its props
#ifdef BUILD_ASSIMP
struct AnimationClip
{
	AnimationClip(const aiAnimation* Assimpanim, float AnimRate = 1.0f)
	{
		AssimpAnim = Assimpanim;
		Rate = AnimRate;
	}
	AnimationClip()
	{}
	float Rate = 1.0f;
	const aiAnimation* AssimpAnim = nullptr;
};
#endif
struct RawMeshData
{
	std::vector<OGLVertex> vertices;
	std::vector<IndType> indices;
	int MaterialIndex = 0;
};
///class that loads mesh data into a Mesh Entity;
class MeshLoader
{
public:
	static const glm::vec3 DefaultScale;
	struct FMeshLoadingSettings
	{
		glm::vec3 Scale = glm::vec3(1, 1, 1);
		glm::vec2 UVScale = glm::vec2(1, 1);
		bool InitOnAllDevices = true;
		bool CreatePhysxMesh = false;
		bool GenerateIndexed = true;
		bool FlipUVs = false;
		void Serialize(Archive* A);
		std::vector<std::string> IgnoredMeshObjectNames;
#if 0//def BUILD_ASSIMP
		AnimationClip AnimSettings;
#endif
		bool AllowInstancing = true;//Temp
	};

	static bool LoadAnimOnly(std::string filename, SkeletalMeshEntry * SkeletalMesh, std::string Name, FMeshLoadingSettings& Settings);
	static bool LoadMeshFromCookedFile(std::string filename, std::vector<RawMeshData*> &Meshes);
	static bool SaveMeshToCookedFile(std::string filename, std::vector<RawMeshData*> &Meshes);
	static bool LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<MeshEntity*> &Meshes, SkeletalMeshEntry** pSkeletalEntity);
	static bool LoadMeshFromFile_Direct(std::string filename, FMeshLoadingSettings & Settings, std::vector<OGLVertex>& vertices, std::vector<IndType>& indices);
	Mesh* TryLoadFromCache(std::string Path);
	static MeshLoader* Get();
	static void RegisterLoad(std::string path, Mesh* mesh);
	static void ShutDown();
	void DestoryMeshes();
private:
	static MeshLoader* Instance;
	std::map<std::string, Mesh*> CreatedMeshes;
};
//#Anim up to 8 
#define NUM_BONES_PER_VEREX 4
struct VertexBoneData
{
	unsigned int IDs[NUM_BONES_PER_VEREX] = { 0 };
	float Weights[NUM_BONES_PER_VEREX] = { 0.0f };
	void AddBoneData(uint BoneID, float Weight);
};

struct BoneInfo
{
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransformation;

	BoneInfo()
	{}
};

struct SkeletalMeshEntry
{
	SkeletalMeshEntry(aiAnimation* anim);

	float CurrnetTime = 0.0f;
	float GetMaxTime()
	{
		return MaxTime;
	};
	void RenderBones(Transform* T);
	void Tick(float Delta);
	void PlayAnimation(std::string name);
	void LoadBones(uint MeshIndex, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones);
	const aiNodeAnim * FindNodeAnim(const aiAnimation * pAnimation, const std::string NodeName);
	std::map<std::string, uint> m_BoneMapping;
	std::vector<BoneInfo> m_BoneInfo;
	std::vector<MeshEntity*> MeshEntities;
	std::vector<glm::mat4x4> FinalBoneTransforms;
	void InitScene(const aiScene* sc);
	uint FindPosition(float AnimationTime, const aiNodeAnim * pNodeAnim);
	void Release();
#ifdef BUILD_ASSIMP
	void SetAnim(const AnimationClip& anim);
	std::map<std::string, AnimationClip> AnimNameMap;
private:
	const aiScene* Scene = nullptr;
	uint FindRotation(float AnimationTime, const aiNodeAnim * pNodeAnim);
	uint FindScaling(float AnimationTime, const aiNodeAnim * pNodeAnim);
	void CalcInterpolatedScaling(aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion & Out, float AnimationTime, const aiNodeAnim * pNodeAnim);
	void ReadNodes(float time, const aiNode* pNode, const glm::mat4 ParentTransfrom, const aiAnimation* Anim);
	AnimationClip CurrentAnim;
#endif
	float MaxTime = 0.0f;
	int m_NumBones = 0;
	glm::mat4 ModelInvTransfrom;
};