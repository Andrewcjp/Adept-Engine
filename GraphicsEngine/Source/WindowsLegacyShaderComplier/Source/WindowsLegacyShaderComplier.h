#pragma once
#include "RHI/ShaderBase.h"

class D3D12Shader;
struct ShaderComplieItem;
class WindowsLegacyShaderComplier
{
public:
	WindowsLegacyShaderComplier();
	~WindowsLegacyShaderComplier();
	void Init();
	std::string GetShaderModelString(D3D_SHADER_MODEL Clamp);
	std::string GetComplieTarget(EShaderType::Type t);
	void ComplieShader(ShaderComplieItem* Shader);

	IDxcCompiler* Complier = nullptr;

};

