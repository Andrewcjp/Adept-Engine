#pragma once
#include "Rendering/Core/RenderBaseTypes.h"
#include "assimp/vector3.h"
#include "assimp/quaternion.h"

struct MeshEntity;
struct aiScene;
class Archive;
struct SkeletalMeshEntry;
struct aiMesh;
struct aiAnimation;
struct aiNode;
struct aiAnimation;
struct aiNodeAnim;

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
	};
	static bool LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<MeshEntity*> &Meshes, SkeletalMeshEntry** pSkeletalEntity);
	static bool LoadMeshFromFile_Direct(std::string filename, FMeshLoadingSettings & Settings, std::vector<OGLVertex>& vertices, std::vector<int>& indices);
};
#define NUM_BONES_PER_VEREX 4
struct VertexBoneData
{
	unsigned int IDs[NUM_BONES_PER_VEREX] = {0};
	float Weights[NUM_BONES_PER_VEREX] = {0.0f};
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
	float GetMaxTime() { return MaxTime; };
	void Tick(float Delta);
	void LoadBones(uint MeshIndex, const aiMesh * pMesh, std::vector<VertexBoneData>& Bones, int BaseVertex);
	const aiNodeAnim * FindNodeAnim(const aiAnimation * pAnimation, const std::string NodeName);
	std::map<std::string, uint> m_BoneMapping;
	std::vector<BoneInfo> m_BoneInfo;
	std::vector<MeshEntity*> MeshEntities;
	std::vector<glm::mat4x4> FinalBoneTransforms;
	void InitScene(const aiScene* sc);
	uint FindPosition(float AnimationTime, const aiNodeAnim * pNodeAnim);
private:
	const aiScene* Scene = nullptr;
	uint FindRotation(float AnimationTime, const aiNodeAnim * pNodeAnim);
	uint FindScaling(float AnimationTime, const aiNodeAnim * pNodeAnim);
	void CalcInterpolatedScaling(aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim);
	void CalcInterpolatedPosition(aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion & Out, float AnimationTime, const aiNodeAnim * pNodeAnim);
	void ReadNodes(float time, const aiNode* pNode, const glm::mat4 ParentTransfrom);
	float MaxTime = 0.0f;
	int m_NumBones = 0;
	glm::mat4 ModelInvTransfrom;
};