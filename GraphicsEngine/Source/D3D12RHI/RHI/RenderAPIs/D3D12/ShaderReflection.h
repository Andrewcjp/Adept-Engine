#pragma once
#include "RHI\Shader.h"
#include "D3D12RHI.h"
#include <d3d12shader.h>

class ShaderReflection
{
public:
	static void GatherRSBinds(ShaderBlob* target,std::vector<ShaderParameter> & shaderbinds,bool & iscompute);
	static ShaderParameter ConvertParm(D3D12_SHADER_INPUT_BIND_DESC & desc);
};

