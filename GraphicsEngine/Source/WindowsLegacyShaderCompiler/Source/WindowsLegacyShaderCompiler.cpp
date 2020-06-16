#include "WindowsLegacyShaderCompiler.h"
#include "RHI/ShaderCompilerModule.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/AssetManager.h"
#include <d3dcompiler.h>

WindowsLegacyShaderCompiler::WindowsLegacyShaderCompiler()
{
	Init();
}


WindowsLegacyShaderCompiler::~WindowsLegacyShaderCompiler()
{}

void WindowsLegacyShaderCompiler::Init()
{
	//DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
}

std::string ConvertToLevelString(D3D_SHADER_MODEL SM)
{
	switch (SM)
	{
	case D3D_SHADER_MODEL_5_1:
		return "_6_0";//dxil does not support 5_1 profiles
	case D3D_SHADER_MODEL_6_0:
		return "_6_0";
	case D3D_SHADER_MODEL_6_1:
		return "_6_1";
#if WIN10_1809
	case D3D_SHADER_MODEL_6_2:
		return "_6_2";
	case D3D_SHADER_MODEL_6_3:
		return "_6_3";
	case D3D_SHADER_MODEL_6_4:
		return "_6_4";
	case D3D_SHADER_MODEL_6_5:
		return "_6_5";
#endif
	}
	return "BAD!";
}
std::string WindowsLegacyShaderCompiler::GetShaderModelString(D3D_SHADER_MODEL Clamp)
{
	/*D3D12DeviceContext* Con = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	D3D_SHADER_MODEL SM = Con->GetShaderModel();*/
	D3D_SHADER_MODEL SM = D3D_SHADER_MODEL_5_1;

	if (SM > Clamp)
	{
		SM = Clamp;
	}
	return "_5_1";// ConvertToLevelString(SM);
}

std::string WindowsLegacyShaderCompiler::GetComplieTarget(EShaderType::Type t)
{
#if WIN10_1809 
	D3D_SHADER_MODEL ClampSm = D3D_SHADER_MODEL_6_3;
#else
	D3D_SHADER_MODEL ClampSm = D3D_SHADER_MODEL_5_1;
#endif
	switch (t)
	{
	case EShaderType::SHADER_COMPUTE:
		return "cs" + GetShaderModelString(ClampSm);
	case EShaderType::SHADER_VERTEX:
		return "vs" + GetShaderModelString(ClampSm);
	case EShaderType::SHADER_FRAGMENT:
		//Currently there is no PS_6_4 target
		return "ps" + GetShaderModelString(Math::Min(ClampSm, D3D_SHADER_MODEL_6_3));
	case EShaderType::SHADER_GEOMETRY:
		return "gs" + GetShaderModelString(ClampSm);
#if WIN10_1809
	case EShaderType::SHADER_RT_LIB:
		return "lib" + GetShaderModelString(ClampSm);
#endif
	}
	AD_Assert_Always("Missing Shader profile!");
	return "";
}

LPCWSTR GetCopyStr(std::string data)
{
	std::wstring t = StringUtils::ConvertStringToWide(data);
	wchar_t* Data = new wchar_t[t.size() + 1];
	t.copy(Data, t.size());
	Data[t.size()] = L'\0';
	return Data;
}

D3D_SHADER_MACRO* ParseDefines(ShaderCompileItem * Shader)
{
	if (Shader->Defines.size() == 0)
	{
		return nullptr;
	}
	D3D_SHADER_MACRO* out = new D3D_SHADER_MACRO[Shader->Defines.size() + 1];
	for (int i = 0; i < Shader->Defines.size(); i++)//array is set up as Name, Value
	{
		out[i].Name = Shader->Defines[i].Name.c_str();
		out[i].Definition = Shader->Defines[i].Value.c_str();
	}
	int last = (int)Shader->Defines.size();
	out[last].Definition = NULL;
	out[last].Name = NULL;
	return out;
}

void WindowsLegacyShaderCompiler::ComplieShader(ShaderCompileItem * ShaderItem)
{
	if (ShaderItem->ShaderModel == EShaderSupportModel::SM6)
	{
		ShaderItem->Result = EShaderError::SHADER_ERROR_UNSUPPORTED;
		return;
	}

	ID3DBlob* pErrorBlob = NULL;
	UINT  compileFlags = 0;

	if (ShaderItem->CompileShaderDebug)
	{
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ALL_RESOURCES_BOUND;
	}
	else
	{
		compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_STRICTNESS /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/;
	}
	compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
	ID3DBlob* Outputblob = NULL;
	D3D_SHADER_MACRO* defines = ParseDefines(ShaderItem);
	HRESULT hr = D3DCompile(ShaderItem->Data->Source.c_str(), ShaderItem->Data->Source.size(), ShaderItem->ShaderName.c_str(), defines, nullptr, (ShaderItem->EntryPoint).c_str(), GetComplieTarget(ShaderItem->Stage).c_str(),
		compileFlags, 0, &Outputblob, &pErrorBlob);

	//HRESULT hr = Compiler->Compile(pSource, StringUtils::ConvertStringToWide(ShaderItem->ShaderName).c_str(), StringUtils::ConvertStringToWide(ShaderItem->EntryPoint).c_str(),
	//	GetComplieTarget(ShaderItem->Stage).c_str(), arguments.data(), (UINT)arguments.size(), defs, (UINT)ShaderItem->Defines.size(), nullptr, &R);

	if (pErrorBlob)
	{
		std::string Log = "Shader Compile Output: ";
		Log.append(ShaderItem->ShaderName);
		Log.append("\n");

		std::string S = reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer());
		Log.append(S);

		if (FAILED(hr))
		{
			Log::LogMessage(Log, Log::Severity::Error);
			PlatformApplication::DisplayMessageBox("Shader Complie Error", Log);
			pErrorBlob->Release();
#ifndef NDEBUG
			__debugbreak();
#endif
			Engine::AssertExit(-1);
			ShaderItem->Result = EShaderError::SHADER_ERROR_COMPILE;
			return;
		}
		else
		{
			if (S.length() > 0)
			{
				Log::LogMessage(Log, Log::Severity::Warning);
			}
		}
	}

	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}
	if (FAILED(hr))
	{
		ShaderItem->Result = EShaderError::SHADER_ERROR_CREATE;
		return;
	}

	ShaderItem->Result = EShaderError::SHADER_ERROR_NONE;
	ShaderItem->Blob = new ShaderByteCodeBlob(Outputblob->GetBufferPointer(), Outputblob->GetBufferSize());
}