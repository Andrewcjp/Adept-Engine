#include "WindowsShaderModule.h"
#include "Core\Module\ModuleManager.h"
#include "Source\WindowsShaderComplier.h"


#ifdef WINDOWSSHADERCOMPILER_EXPORT
IMPLEMENT_MODULE_DYNAMIC(WindowsShaderCompilerModule);
#endif

WindowsShaderCompilerModule::WindowsShaderCompilerModule()
{}

WindowsShaderCompilerModule::~WindowsShaderCompilerModule()
{}

void WindowsShaderCompilerModule::ComplieShader(ShaderComplieItem* shader)
{
	Complier->ComplieShader(shader);
}

bool WindowsShaderCompilerModule::StartupModule()
{
	Complier = new WindowsShaderComplier();
	Complier->Init();
	return true;
}

void WindowsShaderCompilerModule::ShutdownModule()
{
	SafeDelete(Complier);
}

bool WindowsShaderCompilerModule::SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderComplieItem* item)
{
	if (SM == EShaderSupportModel::SM5)
	{
		return false;
	}
	return Platform == EPlatforms::Windows || Platform == EPlatforms::Windows_DX12;
}

