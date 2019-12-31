#pragma once

class Shader_Pair;
class VoxelTracingEngine
{
public:
	void VoxliseTest(RHICommandList* list);
	void RenderVoxelDebug(RHICommandList * list, FrameBuffer* buffer);
	VoxelTracingEngine();

	static VoxelTracingEngine * Get();


	RHITexture* VoxelMap = nullptr;
private:
	static VoxelTracingEngine* Instance;
	Shader_Pair* voxeliseShader = nullptr;
	Shader_Pair* DebugvoxeliseShader = nullptr;
	int size = 100;
};

