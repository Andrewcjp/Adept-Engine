#pragma once
#include "RHI\BaseTexture.h"

class Material;
class Asset_Shader;
class Defaults
{
public:
	static void Start();
	static void Shutdown();
	static BaseTextureRef GetDefaultTexture();
	static Material* GetDefaultMaterial();
	static RHIBuffer* GetQuadBuffer();
private:
	Material* DefaultMateral = nullptr;
	Defaults();
	~Defaults();
	static Defaults* Instance;
	BaseTextureRef DefaultTexture;
	RHIBuffer* VertexBuffer = nullptr;
};

