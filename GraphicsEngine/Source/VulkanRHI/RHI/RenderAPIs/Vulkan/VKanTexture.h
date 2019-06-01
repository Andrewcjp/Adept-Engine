#pragma once
#include "RHI/BaseTexture.h"
#if BUILD_VULKAN
#include "Core/Assets/AssetTypes.h"
#include "vulkan/vulkan_core.h"
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
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;

	Descriptor GetDescriptor(int slot);
};

#endif 