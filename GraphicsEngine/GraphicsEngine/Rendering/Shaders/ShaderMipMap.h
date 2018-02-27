#pragma once
#include "..\RHI\Shader.h"
#include "../D3D12/D3D12Texture.h"
class ShaderMipMap :
	public Shader
{
public:
	ShaderMipMap();
	virtual ~ShaderMipMap();
	void GenAllmips();
	void GenerateMipsForTexture( D3D12Texture* tex);
	ID3D12GraphicsCommandList* pCommandList = nullptr;
	std::vector<D3D12Texture*> Targets;
};

