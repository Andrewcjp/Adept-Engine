#include "VKNTexture.h"
#pragma optimize("O3",on)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma optimize("",off)
#include "vulkan/vulkan_core.h"
#include "VKNRHI.h"
#include "VKNDeviceContext.h"
#include "Core/Assets/AssetManager.h"
#include "VKNHelpers.h"
#include "Descriptor.h"
#include "gli/gli.hpp"

#if BUILD_VULKAN

VKNTexture::VKNTexture()
{

}

VKNTexture::~VKNTexture()
{}

bool VKNTexture::CreateFromFile(AssetPathRef FileName)
{
	//int texWidth, texHeight, texChannels;
	std::string FilePAth = FileName.GetFullPathToAsset();
	//stbi_uc* pixels = stbi_load(FilePAth.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	//if (pixels == nullptr)
	//{
	//	return true;
	//}

	gli::texture tex = gli::load(FilePAth);
	if (tex.empty())
	{
		return false;
	}
	texture = &tex;

	Log::LogMessage("Loading texture " + FilePAth);

	Description.Width = tex.extent().x;
	Description.Height = tex.extent().y;
	Description.MipLevels = tex.levels();
	Description.BitDepth = 4;// texChannels;
	Description.PtrToData = tex.data();
	Description.Faces = tex.faces();
	Description.ImageByteSize = tex.size();
	if (tex.target() == gli::TARGET_CUBE)
	{
		///		__debugbreak();
		Description.TextureType = ETextureType::Type_CubeMap;
		CurrentTextureType = ETextureType::Type_CubeMap;
	}
	CreateTextureFromDesc(Description);
	//stbi_image_free(pixels);
	TexturePath = FileName.GetRelativePathToAsset();
	return true;
}

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
	gli::texture tex;
	if (texture != nullptr)
	{
		tex = *texture;
	}
	
	Description = desc;
	VkDeviceSize imageSize = desc.Width * desc.Height * desc.BitDepth;
	if (desc.ImageByteSize != 0)
	{
		imageSize = desc.ImageByteSize;
	}
	VKNDeviceContext* D = (VKNDeviceContext*)RHI::GetDefaultDevice();
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
	VKNHelpers::transitionImageLayout(B, textureImage, fmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,desc.MipLevels,desc.Faces);
	if (texture == nullptr)
	{
		VKNHelpers::copyBufferToImage(B, stagingBuffer, textureImage, static_cast<uint32_t>(desc.Width), static_cast<uint32_t>(desc.Height));
	}
	else
	{
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t face = 0; face < tex.faces(); face++)
		{
			for (uint32_t level = 0; level < tex.levels(); level++)
			{
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = tex.extent(level).x;
				bufferCopyRegion.imageExtent.height = tex.extent(level).y;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopyRegion);

				// Increase offset into staging buffer for next level / face
				offset += tex.size(level);
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
	}
	VKNHelpers::transitionImageLayout(B, textureImage, fmt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,desc.MipLevels, desc.Faces);
	UpdateSRV();
}

Descriptor VKNTexture::GetDescriptor(int slot)
{
	Descriptor D = Descriptor(EDescriptorType::SRV);
	D.Texture = this;
	D.bindpoint = slot;
	return D;
}

#endif