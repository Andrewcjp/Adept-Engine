#include "ShaderReflection.h"


#include "Core\Utils\VectorUtils.h"
#include "Core\Assets\AssetManager.h"
#define DXIL_FOURCC(ch0, ch1, ch2, ch3) (                            \
  (uint32_t)(uint8_t)(ch0)        | (uint32_t)(uint8_t)(ch1) << 8  | \
  (uint32_t)(uint8_t)(ch2) << 16  | (uint32_t)(uint8_t)(ch3) << 24   \
  )
void ShaderReflection::GatherRSBinds(ShaderBlob* target, EShaderType::Type Type, std::vector<ShaderParameter> & shaderbinds, bool & iscompute, ShaderSourceFile* Sourcefile, D3D12Shader* shader)
{
	ID3D12ShaderReflection* REF = nullptr;
#if !USE_DIXL
	ThrowIfFailed(D3DReflect(target->GetBufferPointer(), target->GetBufferSize(), ID_PASS(&REF)));
	RelfectShader(REF, iscompute, shaderbinds, Type, shader);
#else
	IDxcContainerReflection* pReflection;

	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));
	HRESULT res = S_OK;
#ifdef PLATFORM_WINDOWS
	UINT32 shaderIdx = -1;
	pReflection->Load(target);
	if (res == S_OK)
	{
		ThrowIfFailed(pReflection->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'), &shaderIdx));
	}
	ensure(shaderIdx > 0);
#endif
	D3D12_LIBRARY_DESC LDesc;
	if (Type == EShaderType::SHADER_RT_LIB)
	{
		ID3D12LibraryReflection* Libreflect;
#ifdef PLATFORM_WINDOWS
		ThrowIfFailed(pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&Libreflect)));
#else
		IDxcUtils* pUtils;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
		DxcBuffer ReflectionData;
		ReflectionData.Encoding = DXC_CP_ACP;
		ReflectionData.Ptr = target->GetBufferPointer();
		ReflectionData.Size = target->GetBufferSize();
		pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&Libreflect));
#endif
		Libreflect->GetDesc(&LDesc);

		for (uint i = 0; i < LDesc.FunctionCount; i++)
		{
			ID3D12FunctionReflection* PL = Libreflect->GetFunctionByIndex(i);
			RelfectShaderFromLib(PL, shaderbinds);
		}
	}
	else
	{
#ifdef PLATFORM_WINDOWS
		ThrowIfFailed(pReflection->GetPartReflection(shaderIdx, __uuidof(ID3D12ShaderReflection), (void**)&REF));
#else
		IDxcUtils* pUtils;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
		DxcBuffer ReflectionData;
		ReflectionData.Encoding = DXC_CP_ACP;
		ReflectionData.Ptr = target->GetBufferPointer();
		ReflectionData.Size = target->GetBufferSize();
		pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&REF));
#endif

		RelfectShader(REF, iscompute, shaderbinds, Type, shader);
	}
#endif
	ApplyRootConstantPatch(REF, shaderbinds, Sourcefile);
}

void ShaderReflection::ApplyRootConstantPatch(ID3D12ShaderReflection* REF, std::vector<ShaderParameter> & shaderbinds, ShaderSourceFile* Sourcefile)
{
	if (Sourcefile == nullptr)
	{
		return;
	}
	for (uint32 i = 0; i < Sourcefile->RootConstants.size(); i++)
	{
		for (uint32 x = 0; x < shaderbinds.size(); x++)
		{
			if (shaderbinds[x].Name == Sourcefile->RootConstants[i])
			{
				ensure(shaderbinds[x].Type == ShaderParamType::CBV || shaderbinds[x].Type == ShaderParamType::RootConstant || shaderbinds[x].Type == ShaderParamType::Buffer);
				shaderbinds[x].Type = ShaderParamType::RootConstant;
				ID3D12ShaderReflectionVariable* vr = REF->GetVariableByName(shaderbinds[x].Name.c_str());
				if (vr != nullptr)
				{
					D3D12_SHADER_VARIABLE_DESC VR_desc = {};
					HRESULT hr = vr->GetDesc(&VR_desc);
					if (!FAILED(hr))
					{
						//a single root constant is 32 bit == 4 bytes so:
						shaderbinds[x].NumVariablesContained = VR_desc.Size / 4;
						break;
					}
				}
				ID3D12ShaderReflectionConstantBuffer* VCB = REF->GetConstantBufferByName(shaderbinds[x].Name.c_str());
				if (VCB != nullptr)
				{
					D3D12_SHADER_BUFFER_DESC d;
					if (!FAILED(VCB->GetDesc(&d)))
					{
						int RawSize = 0;// all CBs are aligned to 16 bytes
						for (uint vindex = 0; vindex < d.Variables; vindex++)
						{
							vr = VCB->GetVariableByIndex(vindex);
							D3D12_SHADER_VARIABLE_DESC VR_desc = {};
							if (!FAILED(vr->GetDesc(&VR_desc)))
							{
								RawSize += VR_desc.Size;
							}
						}
						shaderbinds[x].NumVariablesContained = RawSize / 4;
						break;
					}
				}
				LogEnsure_Always("Failed to Reflect shader var " + shaderbinds[x].Name);
			}
		}
	}
}

void ShaderReflection::RelfectShaderFromLib(ID3D12FunctionReflection* REF, std::vector<ShaderParameter> & shaderbinds)
{
	D3D12_FUNCTION_DESC desc;
	REF->GetDesc(&desc);
	for (unsigned int i = 0; i < desc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC TMPdesc;
		REF->GetResourceBindingDesc(i, &TMPdesc);
		//todo: restrict visibility and then detect duplicates and adjust visibility
		ShaderParameter p = ConvertParm(TMPdesc);
		if (p.Type != ShaderParamType::Limit && p.Type != ShaderParamType::Sampler)
		{
			VectorUtils::AddUnique(shaderbinds, p);
		}
	}
}
RHI_SHADER_VISIBILITY GetVisForShader(EShaderType::Type Type)
{
	switch (Type)
	{
	case EShaderType::SHADER_VERTEX:
		return SHADER_VISIBILITY_VERTEX;
	case EShaderType::SHADER_FRAGMENT:
		return SHADER_VISIBILITY_PIXEL;
		break;
	case EShaderType::SHADER_GEOMETRY:
		return SHADER_VISIBILITY_GEOMETRY;
	case EShaderType::SHADER_COMPUTE:
		return SHADER_VISIBILITY_ALL;
	case EShaderType::SHADER_HULL:
		break;
	case EShaderType::SHADER_DOMAIN:
		break;
	case EShaderType::SHADER_RT_LIB:
		break;
	}
	return SHADER_VISIBILITY_ALL;
}
void ShaderReflection::RelfectShader(ID3D12ShaderReflection* REF, bool &iscompute, std::vector<ShaderParameter> & shaderbinds, EShaderType::Type Type, D3D12Shader* shader)
{
	if (REF == nullptr)
	{
		return;
	}
	D3D12_SHADER_DESC desc;
	REF->GetDesc(&desc);
	int ProgramType = (desc.Version & 0xFFFF0000) >> 16;
	if (ProgramType == D3D12_SHADER_VERSION_TYPE::D3D12_SHVER_COMPUTE_SHADER)
	{
		iscompute = true;
	}
	for (uint i = 0; i < desc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC TMPdesc;
		REF->GetResourceBindingDesc(i, &TMPdesc);
		ShaderParameter p = ConvertParm(TMPdesc);
		if (Type == EShaderType::SHADER_FRAGMENT && p.Type == ShaderParamType::SRV)
		{
			p.Visiblity = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_PIXEL;
		}
		if (p.Type != ShaderParamType::Limit && p.Type != ShaderParamType::Sampler)//#todo: handle dynamic samplers
		{
			VectorUtils::AddUnique(shaderbinds, p);
		}
	}
	if (iscompute)
	{
		uint x, y, z = 0;
		REF->GetThreadGroupSize(&x, &y, &z);
		shader->ComputeThreadSize = glm::ivec3(x, y, z);
	}
	shader->InstructionCount = desc.InstructionCount;
}

ShaderParameter ShaderReflection::ConvertParm(D3D12_SHADER_INPUT_BIND_DESC& desc)
{
	ShaderParameter S = {};
	S.NumDescriptors = desc.BindCount;
	S.RegisterSlot = desc.BindPoint;
	S.RegisterSpace = desc.Space;
	S.Name = desc.Name;
	S.Visiblity = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL;
	switch (desc.Type)
	{
	case D3D_SIT_CBUFFER:
		S.Type = ShaderParamType::CBV;
		break;
	case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_CONSUME_STRUCTURED:
	case D3D_SHADER_INPUT_TYPE::D3D11_SIT_STRUCTURED:
	case D3D_SHADER_INPUT_TYPE::D3D11_SIT_BYTEADDRESS:
	case D3D_SIT_TEXTURE:
		S.Type = ShaderParamType::SRV;
		break;
	case D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
	case D3D_SIT_UAV_RWSTRUCTURED:
	case D3D11_SIT_UAV_RWTYPED:
	case D3D_SIT_UAV_RWBYTEADDRESS:
		S.Type = ShaderParamType::UAV;
		break;
	case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
		S.Type = ShaderParamType::Sampler;
		break;
	case 12://RaytracingAccelerationStructure
		S.Type = ShaderParamType::AccelerationStructure;
		break;
	default:
		__debugbreak();
		break;

	}
	return S;
}
