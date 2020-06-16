#include "WindowsLegacyShaderCompilerModule.h"
#include "Core\Module\ModuleManager.h"
#include "Source\WindowsLegacyShaderCompiler.h"


#ifdef WINDOWSLEGACYSHADERCOMPILER_EXPORT
IMPLEMENT_MODULE_DYNAMIC(WindowsLegacyShaderCompilerModule);
#endif

WindowsLegacyShaderCompilerModule::WindowsLegacyShaderCompilerModule()
{}

WindowsLegacyShaderCompilerModule::~WindowsLegacyShaderCompilerModule()
{}

void WindowsLegacyShaderCompilerModule::CompileShader(ShaderCompileItem* shader)
{
	Compiler->ComplieShader(shader);
}

bool WindowsLegacyShaderCompilerModule::StartupModule()
{
	Compiler = new WindowsLegacyShaderCompiler();
	Compiler->Init();
	return true;
}

void WindowsLegacyShaderCompilerModule::ShutdownModule()
{
	SafeDelete(Compiler);
}

bool WindowsLegacyShaderCompilerModule::SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderCompileItem* item)
{
	if (SM == EShaderSupportModel::SM6 || item->Stage == EShaderType::SHADER_RT_LIB)
	{
		return false;
	}
	return Platform == EPlatforms::Windows || Platform == EPlatforms::Windows_DX12;
}

