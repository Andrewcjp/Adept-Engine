#pragma once
#include "../RenderNode.h"

class Shader_Pair;
class VoxelReflectionsNode: public RenderNode
{
public:
	VoxelReflectionsNode();
	void OnExecute() override;

	void ExecuteVoxelTracing(FrameBuffer* Target, FrameBuffer* Gbuffer);

	void ExecuteVoxelConeTracing(FrameBuffer * Target, FrameBuffer * Gbuffer);

	NameNode("VoxelReflections");
protected:
	void OnNodeSettingChange() override;


	void OnSetupNode() override;
private:
	Shader_Pair* Traceshader = nullptr;
	Shader_Pair* ConeTraceShader = nullptr;
	RHICommandList* List = nullptr;
	struct ShaderData
	{
		glm::vec3 CameraPos;
		int pad;
		glm::ivec2 res;
		int FrameCount;
	};
	ShaderData DataSet;
	RHIBuffer* CBV = nullptr;
};

