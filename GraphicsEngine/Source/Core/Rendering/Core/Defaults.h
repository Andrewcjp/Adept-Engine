#pragma once
#include "RHI\BaseTexture.h"

class Material;
class Asset_Shader;
class Defaults
{
public:
	static void Start();
	static void Shutdown();
	RHI_API static BaseTextureRef GetDefaultTexture();
	RHI_API static Material* GetDefaultMaterial();
	static RHIBuffer* GetQuadBuffer();

private:
	Material* DefaultMateral = nullptr;
	Defaults();
	~Defaults();
	static Defaults* Instance;
	BaseTextureRef DefaultTexture;
	RHIBuffer* VertexBuffer = nullptr;
};

