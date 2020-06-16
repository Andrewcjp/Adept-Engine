#pragma once
#include "RHI/ShaderCompilerModule.h"

class WindowsLegacyShaderCompiler;
class WindowsLegacyShaderCompilerModule : public IShaderCompiler
{
public:
	WindowsLegacyShaderCompilerModule();
	~WindowsLegacyShaderCompilerModule();


	void CompileShader(ShaderCompileItem* shader) override;
	bool StartupModule() override;
	void ShutdownModule() override;
	WindowsLegacyShaderCompiler* Compiler = nullptr;


	bool SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderCompileItem* item) override;

};

