#pragma once
#include "RHI/Shader.h"
#include "Rendering/Shaders/Shader_Main.h"
class Shader_NodeGraph : public Shader_Main
{
public:
	Shader_NodeGraph(class ShaderGraph* graph);
	~Shader_NodeGraph();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<ShaderParameter> GetShaderParameters() override;
	const std::string GetName() override;
private:
	std::string Matname = "";
	ShaderGraph* Graph = nullptr;
};

