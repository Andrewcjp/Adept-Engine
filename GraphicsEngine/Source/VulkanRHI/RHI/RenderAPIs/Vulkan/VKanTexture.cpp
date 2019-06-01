#include "VKanTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan/vulkan_core.h"
#include "VKanRHI.h"
#include "VkanDeviceContext.h"
#include "Core/Assets/AssetManager.h"
#include "VkanHelpers.h"
#include "Descriptor.h"

#if BUILD_VULKAN

VKanTexture::VKanTexture()
{

}

VKanTexture::~VKanTexture()
{}

bool VKanTexture::CreateFromFile(AssetPathRef FileName)
{
	int texWidth, texHeight, texChannels;
	std::string FilePAth = FileName.GetFullPathToAsset();
	stbi_uc* pixels = stbi_load(FilePAth.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (pixels == nullptr)
	{
		return true;
	}
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	VkanDeviceContext* D = (VkanDeviceContext*)RHI::GetDefaultDevice();
	VkanHelpers::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(D->device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(D->device, stagingBufferMemory);
	stbi_image_free(pixels);

	VkanHelpers::createImage(D, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	VkCommandBuffer B = VKanRHI::RHIinstance->setuplist->CommandBuffer;
	VkanHelpers::transitionImageLayout(B, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VkanHelpers::copyBufferToImage(B, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	VkanHelpers::transitionImageLayout(B, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	UpdateSRV();
	return true;
}

void VKanTexture::CreateAsNull()
{

}

void VKanTexture::UpdateSRV()
{
	VkanDeviceContext* D = (VkanDeviceContext*)RHI::GetDefaultDevice();
	textureImageView = VkanHelpers::createImageView(D, textureImage, VK_FORMAT_R8G8B8A8_UNORM);
}

void VKanTexture::CreateTextureFromDesc(const TextureDescription& desc)
{

}

Descriptor VKanTexture::GetDescriptor(int slot)
{
	Descriptor D = Descriptor(EDescriptorType::SRV);
	D.Texture = this;
	D.bindpoint = slot;
	return D;
}

#endif