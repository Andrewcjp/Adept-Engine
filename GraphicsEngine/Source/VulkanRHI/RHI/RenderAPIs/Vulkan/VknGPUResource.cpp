#include "VulkanRHIPCH.h"
#include "VknGPUResource.h"
#include "VKanCommandlist.h"
#include "VkanHelpers.h"


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

void VknGPUResource::SetState(VKanCommandlist* List, VkImageLayout Layout)
{
	if (Layout == CurrentState)
	{
		return;
	}
	VkanHelpers::transitionImageLayout(*List->GetCommandBuffer(), Image, Format, CurrentState, Layout);
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
