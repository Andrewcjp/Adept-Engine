#pragma once
#include "RHI/Shader.h"
class Shader_SkeletalMesh : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_SkeletalMesh);
	Shader_SkeletalMesh(class DeviceContext* dev);
	~Shader_SkeletalMesh();

	virtual std::vector<ShaderParameter> GetShaderParameters() override;
	virtual std::vector<VertexElementDESC> GetVertexFormat() override;
	void PushBones(std::vector<glm::mat4x4>& bonetrans, RHICommandList* list);
private:
	RHIBuffer* BonesBuffer = nullptr;
	static const int MAX_BONES = 100;
	struct BoneData
	{
		glm::mat4x4 Bones[MAX_BONES];
	};
	BoneData boneD;
};

