#pragma once
#include "RHI/BaseTexture.h"

namespace gli { class texture; }
#if BUILD_VULKAN
#include "Core/Assets/AssetTypes.h"
#include "vulkan/vulkan_core.h"
#include "VKNRHI.h"
#include "Descriptor.h"


class VKNTexture : public BaseTexture
{
public:
	VKNTexture();
	~VKNTexture();

	// Inherited via BaseTexture
	virtual bool CreateFromFile(AssetPathRef FileName) override;
	virtual void CreateAsNull() override;
	virtual void UpdateSRV() override;
	virtual void CreateTextureFromDesc(const TextureDescription& desc) override;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	VkImageView textureImageView = VK_NULL_HANDLE;
	VkFormat fmt;
	gli::texture* texture = nullptr;
};

#endif 