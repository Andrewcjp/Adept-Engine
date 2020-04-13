#pragma once

class RHITexture;
class MeshRendererComponent;
class VoxelBottomLevelAccelerationStructure
{
public:
	VoxelBottomLevelAccelerationStructure();
	~VoxelBottomLevelAccelerationStructure();
	void Init(MeshRendererComponent* object);
	void Build(RHICommandList* list);
	bool IsBuilt()const
	{
		return BuildState;
	};
	DeviceContext* Device = nullptr;
	RHITexture* GetVoxelBuffer() const { return VoxelBuffer; }
	glm::ivec3 GetMapSize() const { return MapSize; }
	RHIBuffer* VxControlData = nullptr;
private:
	bool BuildState = false;
	RHITexture* VoxelBuffer = nullptr;
	MeshRendererComponent* TargetMesh = nullptr;
	glm::ivec3 MapSize = glm::ivec3(10, 10, 10);
	RHIBuffer* PrimitiveTransfromBuffer = nullptr;
	VXData ControlData;
};

class VoxelBottomLevelAccelerationStructureInstance 
{
public:
	void SetTransForm(GameObject* GO);

	VoxelBottomLevelAccelerationStructure* AcclerationData = nullptr;
	GameObject* LinkedObject = nullptr;
};

