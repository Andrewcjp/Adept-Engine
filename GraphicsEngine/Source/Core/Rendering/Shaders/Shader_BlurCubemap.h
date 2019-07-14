#pragma once
#include "RHI\Shader.h"
#include "..\Core\Mesh.h"

class ReflectionProbe;
class RHICommandList;
class RHIBuffer;

class Shader_BlurCubemap : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_BlurCubemap);
	Shader_BlurCubemap(DeviceContext* Dev);
	~Shader_BlurCubemap();
	void Init();
	void Update();

	

	struct SData
	{
		float Roughness;
		int Mip;
		glm::ivec2 Threads;
	};
	SData Data[6];

	virtual std::vector<ShaderParameter> GetShaderParameters() override;

private:
	RHIBuffer* ShaderData = nullptr;
	Mesh* Cube = nullptr;
};

