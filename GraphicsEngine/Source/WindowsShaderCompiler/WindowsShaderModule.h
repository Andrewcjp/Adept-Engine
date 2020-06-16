#pragma once
#include "RHI/ShaderCompilerModule.h"

class WindowsShaderCompiler;
class WindowsShaderCompilerModule : public IShaderCompiler
{
public:
	WindowsShaderCompilerModule();
	~WindowsShaderCompilerModule();


	void CompileShader(ShaderCompileItem* shader) override;
	bool StartupModule() override;
	void ShutdownModule() override;
	WindowsShaderCompiler* Compiler = nullptr;


	bool SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderCompileItem* item) override;

};

