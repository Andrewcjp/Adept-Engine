#include "stdafx.h"
#include "D3D12Shader.h"
#include "../Core/Engine.h"
#include <iostream>
#include "D3D12RHI.h"
#include "D3D12CBV.h"
D3D12Shader::D3D12Shader()
{
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}


D3D12Shader::~D3D12Shader()
{
}

void D3D12Shader::CreateShaderProgram()
{
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
EShaderError D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType type)
{
	//convert to LPC 
	std::string path = Engine::GetRootDir();
	path.append("\\asset\\shader\\hlsl\\");
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
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	if (type == SHADER_VERTEX)
	{
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "vs_5_0",
			compileFlags, 0, &m_vsBlob, &pErrorBlob);

	}
	else if (type == SHADER_FRAGMENT)
	{
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "ps_5_0",
			compileFlags, 0, &m_fsBlob, &pErrorBlob);
	}

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			fprintf(stdout, "Shader output: %s\n",
				reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));

			pErrorBlob->Release();
		}
		//D3DEnsure(hr);
		return SHADER_ERROR_COMPILE;
	}

	if (pErrorBlob) pErrorBlob->Release();

	/*if (type == SHADER_VERTEX)
	{
		hr = m_device->CreateVertexShader(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), NULL, &m_vertexShader);
	}
	else if (type == SHADER_FRAGMENT)
	{
		hr = m_device->CreatePixelShader(m_fsBlob->GetBufferPointer(), m_fsBlob->GetBufferSize(), NULL, &m_fragmentShader);
	}*/

	if (FAILED(hr))
	{
		return SHADER_ERROR_CREATE;
	}
	//D3DReflect(m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflector);
	return SHADER_ERROR_NONE;
}

void D3D12Shader::BuildShaderProgram()
{

}

void D3D12Shader::DeleteShaderProgram()
{
}

void D3D12Shader::ActivateShaderProgram()
{
}
void D3D12Shader::ActivateShaderProgram(ID3D12GraphicsCommandList* list)
{
	ThrowIfFailed(list->Reset(m_commandAllocator, m_Shader.m_pipelineState));
}
void D3D12Shader::DeactivateShaderProgram()
{
}

void D3D12Shader::SetUniform1UInt(unsigned int value, const char * param)
{
}

void D3D12Shader::SetAttrib4Float(float f1, float f2, float f3, float f4, const char * param)
{
}

void D3D12Shader::BindAttributeLocation(int index, const char * param_name)
{
}
//ID3D12GraphicsCommandList* D3D12Shader::CreateCommandList()
//{
//	ID3D12GraphicsCommandList* List;
//	ThrowIfFailed(D3D12RHI::Instance->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12RHI::Instance->m_commandAllocator, m_Shader.m_pipelineState, IID_PPV_ARGS(&List)));
//	return  List;
//}
D3D12Shader::PiplineShader D3D12Shader::CreatePipelineShader(D3D12_INPUT_ELEMENT_DESC* inputDisc, int DescCount, ID3DBlob * vsBlob, ID3DBlob * fsBlob)
{
	PiplineShader output;
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(D3D12RHI::Instance->m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 rootParameters[5];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	//	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	//rootParameters[3].InitAsShaderResourceView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	/*	rootSignatureDesc.Init_1_1(1, &rootParameters[0], 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	rootSignatureDesc.Init_1_1(1, &rootParameters[1], 0, nullptr, rootSignatureFlags);*/
	ID3DBlob* signature;
	ID3DBlob* error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(D3D12RHI::Instance->m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&output.m_rootSignature)));

	/*D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};*/


	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = inputDisc;
	psoDesc.InputLayout.NumElements = DescCount;
	psoDesc.pRootSignature = output.m_rootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob);
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(fsBlob);
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(D3D12RHI::Instance->m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&output.m_pipelineState)));

	return output;
}

void D3D12Shader::PushCBVToGPU(ID3D12GraphicsCommandList * list, int offset)
{
	CBV->SetDescriptorHeaps(list);
	CBV->SetGpuView(list, offset);
}
void D3D12Shader::UpdateCBV(SceneConstantBuffer& buffer, int offset)
{
	CBV->UpdateCBV(buffer, offset);
}
void D3D12Shader::InitCBV()
{
	CBV = new D3D12CBV();
	CBV->InitCBV(sizeof(SceneConstantBuffer), 10);
}

CommandListDef * D3D12Shader::CreateShaderCommandList()
{
	if (m_Shader.m_pipelineState == nullptr)
	{
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		m_Shader = CreatePipelineShader(inputElementDescs, _countof(inputElementDescs), m_vsBlob, m_fsBlob);
	}
	CommandListDef* newlist = nullptr;
	ThrowIfFailed(D3D12RHI::Instance->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, m_Shader.m_pipelineState, IID_PPV_ARGS(&newlist)));
	ThrowIfFailed(newlist->Close());
	return newlist;
}

ID3D12CommandAllocator* D3D12Shader::GetCommandAllocator()
{
	return m_commandAllocator;
}
