#pragma once
#include "RHI\Shader.h"
class Shader_Compost :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Compost);
	Shader_Compost(class DeviceContext* context);
	virtual ~Shader_Compost();
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
};

class Shader_VROutput :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_VROutput);
	Shader_VROutput(class DeviceContext* context);
	virtual ~Shader_VROutput();
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
};
