#include "WindowsShaderComplier.h"
#include "RHI/ShaderComplierModule.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/AssetManager.h"

WindowsShaderComplier::WindowsShaderComplier()
{
	Init();
}


WindowsShaderComplier::~WindowsShaderComplier()
{}

void WindowsShaderComplier::Init()
{
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Complier));
}

std::wstring ConvertToLevelString(D3D_SHADER_MODEL SM)
{
	switch (SM)
	{
	case D3D_SHADER_MODEL_5_1:
		return L"_6_0";//dxil does not support 5_1 profiles
	case D3D_SHADER_MODEL_6_0:
		return L"_6_0";
	case D3D_SHADER_MODEL_6_1:
		return L"_6_1";
#if WIN10_1809
	case D3D_SHADER_MODEL_6_2:
		return L"_6_2";
	case D3D_SHADER_MODEL_6_3:
		return L"_6_3";
	case D3D_SHADER_MODEL_6_4:
		return L"_6_4";
	case D3D_SHADER_MODEL_6_5:
		return L"_6_5";
#endif
	}
	return L"BAD!";
}
std::wstring WindowsShaderComplier::GetShaderModelString(D3D_SHADER_MODEL Clamp)
{
	/*D3D12DeviceContext* Con = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	D3D_SHADER_MODEL SM = Con->GetShaderModel();*/
	D3D_SHADER_MODEL SM = D3D_SHADER_MODEL_6_5;

	if (SM > Clamp)
	{
		SM = Clamp;
	}
	return ConvertToLevelString(SM);
}

std::wstring WindowsShaderComplier::GetComplieTarget(EShaderType::Type t)
{
#if WIN10_1809 
	D3D_SHADER_MODEL ClampSm = D3D_SHADER_MODEL_6_3;
#else
	D3D_SHADER_MODEL ClampSm = D3D_SHADER_MODEL_5_1;
#endif
	switch (t)
	{
	case EShaderType::SHADER_COMPUTE:
		return L"cs" + GetShaderModelString(ClampSm);
	case EShaderType::SHADER_VERTEX:
		return L"vs" + GetShaderModelString(ClampSm);
	case EShaderType::SHADER_FRAGMENT:
		//Currently there is no PS_6_4 target
		return L"ps" + GetShaderModelString(Math::Min(ClampSm, D3D_SHADER_MODEL_6_3));
	case EShaderType::SHADER_GEOMETRY:
		return L"gs" + GetShaderModelString(ClampSm);
#if WIN10_1809
	case EShaderType::SHADER_RT_LIB:
		return L"lib" + GetShaderModelString(ClampSm);
#endif
	}
	AD_Assert_Always("Missing Shader profile!");
	return L"";
}

LPCWSTR GetCopyStr(std::string data)
{
	std::wstring t = StringUtils::ConvertStringToWide(data);
	wchar_t* Data = new wchar_t[t.size() + 1];
	t.copy(Data, t.size());
	Data[t.size()] = L'\0';
	return Data;
}

DxcDefine* WindowsShaderComplier::ParseDefines(ShaderComplieItem * Shader)
{
	if (Shader->Defines.size() == 0)
	{
		return nullptr;
	}
	DxcDefine* out = new DxcDefine[Shader->Defines.size() + 1];
	for (int i = 0; i < Shader->Defines.size(); i++)//array is set up as Name, Value
	{
		out[i].Name = GetCopyStr(Shader->Defines[i].Name);
		out[i].Value = GetCopyStr(Shader->Defines[i].Value);
	}
	int last = (int)Shader->Defines.size();
	out[last].Value = NULL;
	out[last].Name = NULL;
	return out;
}

void WindowsShaderComplier::ComplieShader(ShaderComplieItem * ShaderItem)
{
	IDxcBlob* Outputblob = nullptr;
	IDxcBlobEncoding* pErrorBlob = NULL;
	std::vector<LPCWSTR> arguments;
	if (ShaderItem->ComplieShaderDebug)
	{
		//compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ALL_RESOURCES_BOUND;
		arguments.push_back(L"/Zi");
		arguments.push_back(L"/Od");
	}
	else
	{
		//	compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_STRICTNESS /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/;
		arguments.push_back(L"/O3");
		arguments.push_back(L"/Ges");
	}
	IDxcOperationResult* R;

	DxcDefine* defs = ParseDefines(ShaderItem);
	IDxcLibrary *pLibrary;
	IDxcBlobEncoding *pSource;
	DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void **)&pLibrary);
	pLibrary->CreateBlobWithEncodingFromPinned(ShaderItem->Data->Source.c_str(), ShaderItem->Data->Source.size(), CP_UTF8, &pSource);

	HRESULT hr = Complier->Compile(pSource, StringUtils::ConvertStringToWide(ShaderItem->ShaderName).c_str(), StringUtils::ConvertStringToWide(ShaderItem->EntryPoint).c_str(),
		GetComplieTarget(ShaderItem->Stage).c_str(), arguments.data(), (UINT)arguments.size(), defs, (UINT)ShaderItem->Defines.size(), nullptr, &R);
	R->GetResult(&Outputblob);
	R->GetErrorBuffer(&pErrorBlob);
	R->GetStatus(&hr);
	if (pErrorBlob)
	{
		std::string Log = "Shader Compile Output: ";
		Log.append(ShaderItem->ShaderName);
		Log.append("\n");

		IDxcBlobEncoding *pPrintBlob16;
		// We can use the library to get our preferred encoding.
		pLibrary->GetBlobAsUtf8(pErrorBlob, &pPrintBlob16);
		std::string S = std::string((char*)pErrorBlob->GetBufferPointer(), (int)pPrintBlob16->GetBufferSize());
		Log.append(S);
		pPrintBlob16->Release();

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