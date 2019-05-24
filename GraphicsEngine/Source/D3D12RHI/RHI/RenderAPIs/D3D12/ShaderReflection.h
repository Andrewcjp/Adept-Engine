#pragma once
#include "RHI\Shader.h"
#include "D3D12RHI.h"
#include <d3d12shader.h>
struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
class ShaderReflection
{
public:
	static void GatherRSBinds(ID3DBlob* target,std::vector<ShaderParameter> & shaderbinds,bool & iscompute);
	static ShaderParameter ConvertParm(D3D12_SHADER_INPUT_BIND_DESC & desc);
};

