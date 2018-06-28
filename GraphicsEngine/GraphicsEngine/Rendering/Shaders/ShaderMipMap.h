#pragma once
#include "..\RHI\Shader.h"
#include "RHI/RenderAPIs/D3D12/D3D12Texture.h"
class ShaderMipMap :
	public Shader
{
public:
	ShaderMipMap();
	virtual ~ShaderMipMap();
	void GenAllmips(int limit);
	void GenerateMipsForTexture( D3D12Texture* tex, int maxcount);
	ID3D12GraphicsCommandList* pCommandList = nullptr;
	std::vector<D3D12Texture*> Targets;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
};

