#pragma once
#include "RHI/Shader.h"
class Shader_SkeletalMesh : public Shader
{
public:
	Shader_SkeletalMesh(class DeviceContext* dev);
	~Shader_SkeletalMesh();

	virtual std::vector<ShaderParameter> GetShaderParameters() override;
	virtual std::vector<VertexElementDESC> GetVertexFormat() override;
	void PushBones(std::vector<glm::mat4x4>& bonetrans, RHICommandList* list);
private:
	RHIBuffer* BonesBuffer = nullptr;
	const int MAX_BONES = 100;
};

