#pragma once
#include "RHI/BaseTexture.h"
#if BUILD_VULKAN
#include "Core/Assets/AssetTypes.h"
class VKanTexture : public BaseTexture
{
public:
	VKanTexture();
	~VKanTexture();

	// Inherited via BaseTexture
	virtual bool CreateFromFile(AssetPathRef FileName) override;
	virtual void CreateAsNull() override;
	virtual void UpdateSRV() override;
	virtual void CreateTextureFromDesc(const TextureDescription& desc) override;

};

#endif 