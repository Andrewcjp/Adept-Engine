#pragma once
#include "RHI\Shader.h"
#include "D3D12RHI.h"
#include <d3d12shader.h>
#include "RHI\ShaderBase.h"

class ShaderReflection
{
public:
	static void GatherRSBinds(ShaderBlob* target, EShaderType::Type Type,std::vector<ShaderParameter> & shaderbinds,bool & iscompute);

	static void RelfectShaderFromLib(ID3D12FunctionReflection * REF, std::vector<ShaderParameter>& shaderbinds);

	static void RelfectShader(ID3D12ShaderReflection* REF, bool &iscompute, std::vector<ShaderParameter> & shaderbinds, EShaderType::Type Type);

	static ShaderParameter ConvertParm(D3D12_SHADER_INPUT_BIND_DESC & desc);
};

