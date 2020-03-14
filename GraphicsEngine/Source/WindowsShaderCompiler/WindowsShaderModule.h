#pragma once
#include "RHI/ShaderComplierModule.h"

class WindowsShaderComplier;
class WindowsShaderCompilerModule : public IShaderComplier
{
public:
	WindowsShaderCompilerModule();
	~WindowsShaderCompilerModule();


	void ComplieShader(ShaderComplieItem* shader) override;
	bool StartupModule() override;
	void ShutdownModule() override;
	WindowsShaderComplier* Complier = nullptr;


	bool SupportsPlatform(EPlatforms::Type Platform) override;

};

