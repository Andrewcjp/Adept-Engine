#pragma once
#include "RHI/Shader.h"
class Shader_ColourCorrect : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ColourCorrect);
	Shader_ColourCorrect(class DeviceContext* dev);
	~Shader_ColourCorrect();
	std::vector<ShaderParameter> GetShaderParameters();
	std::vector<VertexElementDESC> GetVertexFormat();
};

