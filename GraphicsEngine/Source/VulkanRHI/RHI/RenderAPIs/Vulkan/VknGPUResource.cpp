#include "VulkanRHIPCH.h"
#include "VknGPUResource.h"
#include "VKNCommandlist.h"
#include "VKNHelpers.h"


VknGPUResource::VknGPUResource()
{}

VknGPUResource::~VknGPUResource()
{}

void VknGPUResource::Init(VkImage image, VkDeviceMemory Memory, VkImageLayout State, VkFormat FMT)
{
	Image = image;
	ImageMemory = Memory;
	CurrentState = State;
	Format = FMT;
}

void VknGPUResource::SetState(VKNCommandlist* List, VkImageLayout Layout)
{
	if (Layout == CurrentState)
	{
		return;
	}
	VKNHelpers::transitionImageLayout(*List->GetCommandBuffer(), Image, Format, CurrentState, Layout, 1, Layers);
	CurrentState = Layout;
}

VkImageLayout VknGPUResource::GetState() const
{
	return CurrentState;
}

void VknGPUResource::UpdateState(VkImageLayout State)
{
	CurrentState = State;
}

VkImage VknGPUResource::GetImage()
{
	return Image;
}

VkFormat VknGPUResource::GetFormat()
{
	return Format;
}
