#pragma once
#include "RHI/Shader.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "../Core/Material.h"


class Shader_NodeGraph : public Shader_Main
{
public:
	Shader_NodeGraph(std::string Shadername);
	void Init();
	~Shader_NodeGraph();
	std::vector<VertexElementDESC> GetVertexFormat() override;
	std::vector<ShaderParameter> GetShaderParameters() override;
	const std::string GetName() override;

	//this might be a placeholder for the shader complier
	bool IsValid() const;
	void SetDefines(std::vector<std::string> &Define);
	std::string ShaderSource = "";
private:
	bool HasComplied = false;
	std::string ShaderFilename = "";

};

