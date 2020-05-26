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
	RHI_API static RHITexture* GetDefaultTexture2();
	static RHIBuffer* GetQuadBuffer();

private:
	Material* DefaultMateral = nullptr;
	Defaults();
	~Defaults();
	static Defaults* Instance;
	BaseTextureRef DefaultTexture;
	RHITexture* DefaultRHITex;
	RHIBuffer* VertexBuffer = nullptr;
};

