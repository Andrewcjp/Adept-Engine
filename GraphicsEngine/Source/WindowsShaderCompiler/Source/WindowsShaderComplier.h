#pragma once
#include "RHI/ShaderBase.h"

class D3D12Shader;
struct ShaderComplieItem;
class WindowsShaderComplier
{
public:
	WindowsShaderComplier();
	~WindowsShaderComplier();
	void Init();
	std::wstring GetShaderModelString(D3D_SHADER_MODEL Clamp);
	std::wstring GetComplieTarget(EShaderType::Type t);
	DxcDefine * ParseDefines(ShaderComplieItem * Shader);
	void ComplieShader(ShaderComplieItem* Shader);

	IDxcCompiler* Complier = nullptr;

};

