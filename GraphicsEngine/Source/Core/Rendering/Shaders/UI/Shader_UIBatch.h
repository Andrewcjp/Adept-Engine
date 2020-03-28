#pragma once
#include "RHI/Shader.h"

class DeviceContext;
class Shader_UIBatch :
	public Shader
{
	struct UnifromData
	{
		glm::mat4x4 Proj;
	};
public:
	DECLARE_GLOBAL_SHADER_ARGS(Shader_UIBatch,int);
	Shader_UIBatch(DeviceContext* dev, int Mode);
	virtual ~Shader_UIBatch();
	std::vector<VertexElementDESC> GetVertexFormat() override;
	void PushTOGPU(RHICommandList * list);
	void UpdateUniforms(glm::mat4x4 Proj);

	const std::string GetName() override;

private:
	UnifromData data;
	RHIBuffer* UniformBuffer = nullptr;
	int Mode = 0;
};

