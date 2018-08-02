#pragma once
#include "RHI/Shader.h"

class ShaderMipMap :
	public Shader
{
public:
	ShaderMipMap();
	virtual ~ShaderMipMap();
#if 0
	void GenAllmips(int limit);
	void GenerateMipsForTexture( D3D12Texture* tex, int maxcount);
	ID3D12GraphicsCommandList* pCommandList = nullptr;
	std::vector<D3D12Texture*> Targets;
#endif
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	virtual bool IsComputeShader() override;
};

