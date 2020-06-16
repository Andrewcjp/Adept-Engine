#pragma once
#include "RHI/ShaderBase.h"

class D3D12Shader;
struct ShaderCompileItem;
class WindowsLegacyShaderCompiler
{
public:
	WindowsLegacyShaderCompiler();
	~WindowsLegacyShaderCompiler();
	void Init();
	std::string GetShaderModelString(D3D_SHADER_MODEL Clamp);
	std::string GetComplieTarget(EShaderType::Type t);
	void ComplieShader(ShaderCompileItem* Shader);

	IDxcCompiler* Compiler = nullptr;

};

