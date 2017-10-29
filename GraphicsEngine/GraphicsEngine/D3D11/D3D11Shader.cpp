#include "EngineGlobals.h"
#if BUILD_D3D11
#include <stdio.h>
#include <vector>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include "D3D11Shader.h"
#include "RHI/RHI.h"
#include <iostream>
#include <intrin.h>
D3D11ShaderProgram::~D3D11ShaderProgram()
{
	DeleteShaderProgram();
}
inline bool exists_test3(const std::string& name)
{
	struct stat buffer;
	if ((stat(name.c_str(), &buffer) == 0))
	{
		return true;
	}
	std::cout << "File Does not exist " << name.c_str() << std::endl;
	return false;
}
EShaderError D3D11ShaderProgram::AttachAndCompileShaderFromFile(const char* shadername, EShaderType type)
{
	//convert to LPC 
	std::string path = "../asset/shader/hlsl/";
	std::string name = shadername;
	path.append(name);
	path.append(".hlsl");
	if (!exists_test3(path))
	{
		//std::cout << " File Does not exist" << path.c_str() << std::endl;
#ifdef  _DEBUG
		__debugbreak();
#endif
		return SHADER_ERROR_NOFILE;
	}


	std::wstring newfile((int)path.size(), 0);
	MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), &newfile[0], (int)path.size());
	LPCWSTR filename = newfile.c_str();


	ID3DBlob* pErrorBlob = NULL;
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	if (type == SHADER_VERTEX)
	{
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "vs_4_0",
			shaderFlags, 0, &m_vsBlob, &pErrorBlob);

	}
	else if (type == SHADER_FRAGMENT)
	{
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "ps_4_0",
			shaderFlags, 0, &m_fsBlob, &pErrorBlob);
	}

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			fprintf(stdout, "Shader output: %s\n",
				reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));

			pErrorBlob->Release();
		}
		D3DEnsure(hr);
		return SHADER_ERROR_COMPILE;
	}

	if (pErrorBlob) pErrorBlob->Release();

	if (type == SHADER_VERTEX)
	{
		hr = m_device->CreateVertexShader(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), NULL, &m_vertexShader);
	}
	else if (type == SHADER_FRAGMENT)
	{
		hr = m_device->CreatePixelShader(m_fsBlob->GetBufferPointer(), m_fsBlob->GetBufferSize(), NULL, &m_fragmentShader);
	}

	if (FAILED(hr))
	{
		return SHADER_ERROR_CREATE;
	}
	//D3DReflect(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);
	return SHADER_ERROR_NONE;
}

void D3D11ShaderProgram::DeleteShaderProgram()
{
	if (m_vertexShader) m_vertexShader->Release();

	if (m_fragmentShader) m_fragmentShader->Release();

	if (m_vsBlob) m_vsBlob->Release();

	if (m_fsBlob) m_fsBlob->Release();
}

void D3D11ShaderProgram::ActivateShaderProgram()
{
	RHI::GetD3DContext()->VSSetShader(m_vertexShader, NULL, 0);
	RHI::GetD3DContext()->PSSetShader(m_fragmentShader, NULL, 0);

}

void D3D11ShaderProgram::DeactivateShaderProgram()
{

}
void D3D11ShaderProgram::CreateShaderProgram()
{
}
void D3D11ShaderProgram::BuildShaderProgram()
{
}
void D3D11ShaderProgram::SetUniform1UInt(unsigned int value, const char * param)
{
}
void D3D11ShaderProgram::SetAttrib4Float(float f1, float f2, float f3, float f4, const char * param)
{
}
void D3D11ShaderProgram::BindAttributeLocation(int index, const char * param_name)
{
}
#endif