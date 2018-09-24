#pragma once
#include "RHI/BaseTexture.h"
#if BUILD_VULKAN
#include "Core/Assets/AssetTypes.h"
class VKanTexture:public BaseTexture
{
public:
	VKanTexture();
	~VKanTexture();

	// Inherited via BaseTexture
	virtual bool CreateFromFile(AssetPathRef FileName) override;
	virtual void CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha) override;
	virtual void CreateTextureFromData(void * data, int type, int width, int height, int bits) override;
	virtual void CreateAsNull() override;
};

#endif