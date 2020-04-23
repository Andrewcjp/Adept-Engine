#pragma once
#include "RHI\Shader.h"
#include "RHI\ShaderBase.h"
class Shader_Pair : public Shader
{
public:
	Shader_Pair(DeviceContext* context, const std::vector<std::string>& Names, const std::vector<EShaderType::Type>& StageList, const std::vector<ShaderProgramBase::Shader_Define>& Defines = std::vector<ShaderProgramBase::Shader_Define>());
	~Shader_Pair();
	static Shader_Pair* CreateShader(std::string VertexName, std::string PixelName, DeviceContext* context = nullptr);
	static Shader_Pair* CreateComputeShader(std::string Compute, DeviceContext* context = nullptr);

	virtual const std::string GetName() override;
	static bool IsPartOfGlobalShaderLibrary;
private:
	void Init();
	std::vector<std::string> Names;
	std::vector<EShaderType::Type> StageList;
	std::string shadername = "";
	std::vector<ShaderProgramBase::Shader_Define> Defines;
};

