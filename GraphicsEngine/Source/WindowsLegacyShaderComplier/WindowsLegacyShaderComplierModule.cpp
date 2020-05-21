#include "WindowsLegacyShaderComplierModule.h"
#include "Core\Module\ModuleManager.h"
#include "Source\WindowsLegacyShaderComplier.h"


#ifdef WINDOWSLEGACYSHADERCOMPLIER_EXPORT
IMPLEMENT_MODULE_DYNAMIC(WindowsLegacyShaderCompilerModule);
#endif

WindowsLegacyShaderCompilerModule::WindowsLegacyShaderCompilerModule()
{}

WindowsLegacyShaderCompilerModule::~WindowsLegacyShaderCompilerModule()
{}

void WindowsLegacyShaderCompilerModule::ComplieShader(ShaderComplieItem* shader)
{
	Complier->ComplieShader(shader);
}

bool WindowsLegacyShaderCompilerModule::StartupModule()
{
	Complier = new WindowsLegacyShaderComplier();
	Complier->Init();
	return true;
}

void WindowsLegacyShaderCompilerModule::ShutdownModule()
{
	SafeDelete(Complier);
}

bool WindowsLegacyShaderCompilerModule::SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderComplieItem* item)
{
	if (SM == EShaderSupportModel::SM6 || item->Stage == EShaderType::SHADER_RT_LIB)
	{
		return false;
	}
	return Platform == EPlatforms::Windows || Platform == EPlatforms::Windows_DX12;
}

