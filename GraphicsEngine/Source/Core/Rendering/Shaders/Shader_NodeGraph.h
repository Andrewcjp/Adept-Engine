#pragma once
#include "RHI/Shader.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "../Core/Material.h"


class Shader_NodeGraph : public Shader_Main
{
public:
	Shader_NodeGraph(std::string Shadername, TextureBindSet* binds);
	~Shader_NodeGraph();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<ShaderParameter> GetShaderParameters() override;
	const std::string GetName() override;
	TextureBindSet* GetBinds();
private:
	std::string Matname = "";
	TextureBindSet* Binds = nullptr;
};

