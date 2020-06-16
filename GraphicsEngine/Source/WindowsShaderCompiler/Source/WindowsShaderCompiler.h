#pragma once
#include "RHI/ShaderBase.h"

class D3D12Shader;
struct ShaderCompileItem;
class WindowsShaderCompiler
{
public:
	WindowsShaderCompiler();
	~WindowsShaderCompiler();
	void Init();
	std::wstring GetShaderModelString(D3D_SHADER_MODEL Clamp);
	std::wstring GetComplieTarget(EShaderType::Type t);
	DxcDefine * ParseDefines(ShaderCompileItem * Shader);
	void ComplieShader(ShaderCompileItem* Shader);

	IDxcCompiler* Compiler = nullptr;

};

