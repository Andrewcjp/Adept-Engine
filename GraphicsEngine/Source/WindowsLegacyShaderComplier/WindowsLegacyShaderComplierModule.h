#pragma once
#include "RHI/ShaderComplierModule.h"

class WindowsLegacyShaderComplier;
class WindowsLegacyShaderCompilerModule : public IShaderComplier
{
public:
	WindowsLegacyShaderCompilerModule();
	~WindowsLegacyShaderCompilerModule();


	void ComplieShader(ShaderComplieItem* shader) override;
	bool StartupModule() override;
	void ShutdownModule() override;
	WindowsLegacyShaderComplier* Complier = nullptr;


	bool SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderComplieItem* item) override;

};

