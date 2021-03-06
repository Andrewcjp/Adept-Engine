#pragma once
#define MAX_STRUCT_COUNT 50
class RHITexture;
class VoxelBottomLevelAccelerationStructureInstance;
class Shader_Pair;
struct GPUVXInstanceDesc 
{
	glm::mat4x4 Transform;
	glm::vec4 Pos;
	glm::ivec4 Size;
	glm::vec4 HalfSize;
	int InstanceIndex = 0;
};
class VoxelTopLevelAccelerationStructure
{
public:
	VoxelTopLevelAccelerationStructure();
	~VoxelTopLevelAccelerationStructure();
	void Init();
	void AddStructure(VoxelBottomLevelAccelerationStructureInstance* Structure);
	void RemoveStructure(VoxelBottomLevelAccelerationStructureInstance* Structure);
	void BuildInstances();
	void Build(RHICommandList* list);
	void BuildInstance(RHICommandList * list, VoxelBottomLevelAccelerationStructureInstance* instance, int Index);
	void GenerateMipMaps(RHICommandList * list);
	DeviceContext* Device = nullptr;
	RHITexture* GetVoxelBuffer() const { return VoxelBuffer; }
	RHIBuffer* VoxelMapControlBuffer = nullptr;
	RHITextureArray* SturctureArray = nullptr;
	RHIBuffer* InstanceBuffer = nullptr;
	RHITexture* VoxelAlphaMap= nullptr;
	static const int MipCount = 5;
private:
	std::vector<VoxelBottomLevelAccelerationStructureInstance*> ContainedInstances;
	RHITexture* VoxelBuffer = nullptr;

	glm::ivec3 MapSize = glm::ivec3(1, 1, 1);
	RHIBuffer* ControlBuffer = nullptr;
	std::vector<GPUVXInstanceDesc> Instances;
	struct GPUcontrolData 
	{
		int InstanceCount = 0;
	};
	GPUcontrolData Controls;
};


