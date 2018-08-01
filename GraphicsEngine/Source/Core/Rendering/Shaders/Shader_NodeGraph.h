#pragma once
#include "RHI/Shader.h"
#include "Rendering/Shaders/Shader_Main.h"
class Shader_NodeGraph : public Shader_Main
{
public:
	Shader_NodeGraph(bool LoadForward = true);
	~Shader_NodeGraph();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
};

