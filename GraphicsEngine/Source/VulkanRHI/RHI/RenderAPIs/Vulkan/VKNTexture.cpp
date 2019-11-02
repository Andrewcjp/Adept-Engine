#include "VKNTexture.h"
#pragma optimize("O3",on)

#pragma optimize("",off)
#include "vulkan/vulkan_core.h"
#include "VKNRHI.h"
#include "VKNDeviceContext.h"
#include "Core/Assets/AssetManager.h"
#include "VKNHelpers.h"
#include "Descriptor.h"
#pragma warning(push,0)
#undef max
#include "gli/gli.hpp"
#include "VKNCommandlist.h"
#pragma warning(pop)
#if BUILD_VULKAN

VKNTexture::VKNTexture()
{

}

VKNTexture::~VKNTexture()
{}


void VKNTexture::CreateAsNull()
{

}

void VKNTexture::UpdateSRV()
{
	VKNDeviceContext* D = (VKNDeviceContext*)RHI::GetDefaultDevice();
	textureImageView = VKNHelpers::createImageView(D, textureImage, fmt, VK_IMAGE_ASPECT_COLOR_BIT, Description);
}

void VKNTexture::CreateTextureFromDesc(const TextureDescription& desc)
{
	Description = desc;
	VkDeviceSize imageSize = desc.Width * desc.Height * desc.BitDepth;
	if (desc.ImageByteSize != 0)
	{
		imageSize = desc.ImageByteSize;
	}
	VKNDeviceContext* D = VKNRHI::VKConv(RHI::GetDefaultDevice());
	VKNHelpers::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(D->device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, desc.PtrToData, static_cast<size_t>(imageSize));
	vkUnmapMemory(D->device, stagingBufferMemory);

	if (desc.BitDepth == 4)
	{
		//fmt = VK_FORMAT_R8G8B8A8_UNORM;
		fmt = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		fmt = VK_FORMAT_R8_UNORM;
	}

	VKNHelpers::createImageDesc(/*D,*/ fmt, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, Description);
	VkCommandBuffer B = VKNRHI::RHIinstance->setuplist->CommandBuffer;
	VKNHelpers::transitionImageLayout(B, textureImage, fmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, desc.MipLevels, desc.Faces);
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint64_t offset = 0;
		for (uint32_t face = 0; face < tex.faces(); face++)
		{
			for (uint32_t level = 0; level < tex.levels(); level++)
			{
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = Description.MipExtents(level).x;
				bufferCopyRegion.imageExtent.height = Description.MipExtents(level).y;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopyRegion);

				// Increase offset into staging buffer for next level / face
				offset += Description.Size(level);
			}
		}
		vkCmdCopyBufferToImage(
			B,
			stagingBuffer,
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);
	
	VKNHelpers::transitionImageLayout(B, textureImage, fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, desc.MipLevels, desc.Faces);
	UpdateSRV();
}

#endif