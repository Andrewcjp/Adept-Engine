#pragma once
#include "vulkan/vulkan_core.h"

class VKanTexture;
class VKanBuffer;
struct EDescriptorType
{
	enum Type
	{
		CBV,
		SRV,
		UAV,
		Limit
	};
};
//handle that is added to command list to build the descriptor set for each draw
class Descriptor
{
public:
	Descriptor() {};
	Descriptor(EDescriptorType::Type type);
	~Descriptor();
	EDescriptorType::Type DescType = EDescriptorType::Limit;
	int bindpoint = 0;
	int Offset = 0;
	VKanBuffer* Buffer = nullptr;
	VKanTexture* Texture = nullptr;
	VkImageView ImageView = VK_NULL_HANDLE;
};

