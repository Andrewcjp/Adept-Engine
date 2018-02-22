#include "stdafx.h"
#include "D3D12Shader.h"
#include "../Core/Engine.h"
#include <iostream>
#include "D3D12RHI.h"
#include "D3D12CBV.h"
#include "../Core/Utils/FileUtils.h"
#include "../Core/Utils/StringUtil.h"
#include "../Core/Utils/WindowsHelper.h"
D3D12Shader::D3D12Shader()
{
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	//D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	//{
	//{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	//};
	//InputDesc = inputElementDescs;
}


D3D12Shader::~D3D12Shader()
{}

void D3D12Shader::CreateShaderProgram()
{}

EShaderError D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType type)
{
	//convert to LPC 
	std::string path = Engine::GetRootDir();
	path.append("\\asset\\shader\\hlsl\\");
	std::string name = shadername;
	path.append(name);
	path.append(".hlsl");
	if (!FileUtils::exists_test3(path))
	{
		//std::cout << " File Does not exist" << path.c_str() << std::endl;
#ifdef  _DEBUG
		__debugbreak();
#endif
		return SHADER_ERROR_NOFILE;
	}
	if (type == SHADER_COMPUTE)
	{
		IsCompute = true;
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
		//todo: To d3dcomplie with text
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "vs_5_0",
			compileFlags, 0, &m_vsBlob, &pErrorBlob);

	}
	else if (type == SHADER_FRAGMENT)
	{
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "ps_5_0",
			compileFlags, 0, &m_fsBlob, &pErrorBlob);
	}
	else if (type == SHADER_COMPUTE)
	{
		hr = D3DCompileFromFile(filename, NULL, NULL, "main", "cs_5_0",
			compileFlags, 0, &m_csBlob, &pErrorBlob);
	}

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			fprintf(stdout, "Shader output: %s\n",
				reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			std::string Log = "Shader: ";
			Log.append(StringUtils::ConvertWideToString(filename));
			Log.append("\n");
			Log.append(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			WindowsHelpers::DisplayMessageBox("Shader Complie Error", Log);
			pErrorBlob->Release();
		}
		//D3DEnsure(hr);
		return SHADER_ERROR_COMPILE;
	}

	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}
	if (FAILED(hr))
	{
		return SHADER_ERROR_CREATE;
	}
	return SHADER_ERROR_NONE;
}

void D3D12Shader::BuildShaderProgram()
{
	Init();
}

void D3D12Shader::DeleteShaderProgram()
{}

void D3D12Shader::ActivateShaderProgram()
{}
void D3D12Shader::ActivateShaderProgram(ID3D12GraphicsCommandList* list)
{
	ThrowIfFailed(list->Reset(m_commandAllocator, m_Shader.m_pipelineState));
}
void D3D12Shader::DeactivateShaderProgram()
{}

void D3D12Shader::SetUniform1UInt(unsigned int, const char *)
{}

void D3D12Shader::SetAttrib4Float(float, float, float, float, const char *)
{}

void D3D12Shader::BindAttributeLocation(int, const char *)
{}
D3D12Shader::PiplineShader D3D12Shader::CreatePipelineShader(D3D12_INPUT_ELEMENT_DESC* inputDisc, int DescCount, ID3DBlob * vsBlob, ID3DBlob * fsBlob)
{
	PiplineShader output;
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(D3D12RHI::Instance->m_Primarydevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

	CD3DX12_ROOT_PARAMETER1 rootParameters[5];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	//	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	//rootParameters[3].InitAsShaderResourceView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	D3D12_STATIC_SAMPLER_DESC samplers[2];

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;

	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplers[0] = sampler;


	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 1;
	sampler.RegisterSpace = 0;
	samplers[1] = sampler;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(rootParameters), rootParameters, 2, &samplers[0], D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	ID3DBlob* error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&output.m_rootSignature)));


	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = inputDisc;
	psoDesc.InputLayout.NumElements = DescCount;
	psoDesc.pRootSignature = output.m_rootSignature;

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob);
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(fsBlob);

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = DepthTest;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&output.m_pipelineState)));

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
void D3D12Shader::Init()
{
	if (m_Shader.m_pipelineState == nullptr && !IsCompute)
	{
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		if (InputDesc == nullptr)
		{
			InputDesc = inputElementDescs;
		}
		m_Shader = CreatePipelineShader(InputDesc, Length, m_vsBlob, m_fsBlob);
	}
	if (IsCompute)
	{
		CreateComputePipelineShader();
	}
}

void D3D12Shader::CreateComputePipelineShader()
{
	
	// Compute root signature.
	{

		//The compute shader expects 2 floats, the source texture and the destination texture
		CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2];
		CD3DX12_ROOT_PARAMETER rootParameters[3];
		srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
		rootParameters[0].InitAsConstants(2, 0);
		rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
		rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

		//Static sampler used to get the linearly interpolated color for the mipmaps
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		CD3DX12_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
		//computeRootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr);

		ID3DBlob* signature;
		ID3DBlob* error;
		computeRootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ThrowIfFailed(D3D12SerializeRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_Shader.m_rootSignature)));
		NAME_D3D12_OBJECT(m_Shader.m_rootSignature);
	}
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = m_Shader.m_rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(m_csBlob);
	ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_Shader.m_pipelineState)));
}
CommandListDef * D3D12Shader::CreateShaderCommandList(int device)
{
	Init();
	CommandListDef* newlist = nullptr;
	if (device == 0 || D3D12RHI::Instance->m_Secondarydevice == nullptr)
	{
		ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, m_Shader.m_pipelineState, IID_PPV_ARGS(&newlist)));
	}
	else
	{
		ThrowIfFailed(D3D12RHI::Instance->m_Secondarydevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, m_Shader.m_pipelineState, IID_PPV_ARGS(&newlist)));
	}
	ThrowIfFailed(newlist->Close());
	return newlist;
}

ID3D12CommandAllocator* D3D12Shader::GetCommandAllocator()
{
	return m_commandAllocator;
}

void D3D12Shader::ResetList(ID3D12GraphicsCommandList* list)
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(GetCommandAllocator()->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(list->Reset(GetCommandAllocator(), m_Shader.m_pipelineState));

	// Set necessary state.
	list->SetGraphicsRootSignature(m_Shader.m_rootSignature);


	//m_constantBufferData.M = glm::translate(glm::vec3(0, -5, 0));
	//testshader->UpdateCBV(m_constantBufferData);
	//testshader->PushCBVToGPU(m_commandList, 0);
}
void D3D12Shader::SetInputDesc(D3D12_INPUT_ELEMENT_DESC * desc, int size)
{
	InputDesc = desc;
	Length = size;
}
