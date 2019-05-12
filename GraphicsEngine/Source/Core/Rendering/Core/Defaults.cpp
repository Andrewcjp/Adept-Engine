#include "Stdafx.h"
#include "Defaults.h"
#include "Core\Assets\AssetManager.h"
#include "Core\Platform\PlatformCore.h"
#include "Core\IRefCount.h"
#include "Core\Assets\Asset_Shader.h"

Defaults* Defaults::Instance = nullptr;

Defaults::Defaults()
{
	DefaultTexture = AssetManager::DirectLoadTextureAsset("\\texture\\T_GridSmall_01_D.png");
	ensureFatalMsgf(DefaultTexture != nullptr, "Failed to Load Fallback Texture");
	DefaultShaderMat = new Asset_Shader(true);
}

Defaults::~Defaults()
{
	SafeDelete(DefaultShaderMat);
}

void Defaults::Start()
{
	Instance = new Defaults();
}

void Defaults::Shutdown()
{
	SafeDelete(Instance);
}

BaseTextureRef Defaults::GetDefaultTexture()
{
	return Instance->DefaultTexture;
}

Material * Defaults::GetDefaultMaterial()
{
	return Instance->DefaultShaderMat->GetMaterial();
}

Asset_Shader * Defaults::GetDefaultShaderAsset()
{
	return Instance->DefaultShaderMat;
}


