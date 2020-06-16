#include "WindowsShaderModule.h"
#include "Core\Module\ModuleManager.h"
#include "Source\WindowsShaderCompiler.h"


#ifdef WINDOWSSHADERCOMPILER_EXPORT
IMPLEMENT_MODULE_DYNAMIC(WindowsShaderCompilerModule);
#endif

WindowsShaderCompilerModule::WindowsShaderCompilerModule()
{}

WindowsShaderCompilerModule::~WindowsShaderCompilerModule()
{}

void WindowsShaderCompilerModule::CompileShader(ShaderCompileItem* shader)
{
	Compiler->ComplieShader(shader);
}

bool WindowsShaderCompilerModule::StartupModule()
{
	Compiler = new WindowsShaderCompiler();
	Compiler->Init();
	return true;
}

void WindowsShaderCompilerModule::ShutdownModule()
{
	SafeDelete(Compiler);
}

bool WindowsShaderCompilerModule::SupportsPlatform(EPlatforms::Type Platform, EShaderSupportModel::Type SM, ShaderCompileItem* item)
{
	if (SM == EShaderSupportModel::SM5)
	{
		return false;
	}
	return Platform == EPlatforms::Windows || Platform == EPlatforms::Windows_DX12;
}

