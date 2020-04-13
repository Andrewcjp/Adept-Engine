#pragma once

class Shader_Pair;
struct VXData
{
	glm::vec3 VoxelGridCenter = glm::vec3(0, 0, 0);
	float VoxelSize = 0.1;
	float VoxelSize_INV = 1.0 / VoxelSize;
	glm::ivec3 VoxelRes = glm::ivec3(50, 50, 50);
	glm::vec3 VoxelRes_INV = glm::vec3(1.0f / VoxelRes.x, 1.0f / VoxelRes.y, 1.0f / VoxelRes.z);
	void Update()
	{
		VoxelRes_INV = glm::vec3(1.0f / VoxelRes.x, 1.0f / VoxelRes.y, 1.0f / VoxelRes.z);
		VoxelSize_INV = 1.0 / VoxelSize;
	}
};
class VoxelTracingEngine
{
public:
	void VoxliseTest(RHICommandList* list);
	void RenderVoxelDebug(RHICommandList * list, FrameBuffer* buffer, RHITexture* Texture, RHIBuffer* CBV);
	VoxelTracingEngine();

	static VoxelTracingEngine * Get();

	VXData ControlData;
	RHITexture* VoxelMap = nullptr;
	Shader_Pair* voxeliseShader = nullptr;
	RHIBuffer* VxControlData = nullptr;
private:
	static VoxelTracingEngine* Instance;
	Shader_Pair* DebugvoxeliseShader = nullptr;
	int size = 100;
};

