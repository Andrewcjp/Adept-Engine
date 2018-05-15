#include "stdafx.h"
#include "VKanRHI.h"
#include <stdio.h>
#include <iostream>
#if BUILD_VULKAN
//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>


VKanRHI::VKanRHI(HINSTANCE win32inst)
{
	win32Hinst = win32inst;
}

VKanRHI::~VKanRHI()
{}
void VKanRHI::InitDevice()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << extensionCount << " extensions supported" << std::endl;

	vk::ApplicationInfo const app = vk::ApplicationInfo()
		.setApplicationVersion(0)
		.setEngineVersion(0)
		.setApiVersion(VK_API_VERSION_1_0);
	vk::Result res;


	extension_names [0] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
	extension_names[1] = VK_KHR_SURFACE_EXTENSION_NAME;
	vk::InstanceCreateInfo const inst_info = vk::InstanceCreateInfo().setPApplicationInfo(&app).
		setEnabledExtensionCount(2)
		.setEnabledLayerCount(0)
		.setPpEnabledExtensionNames(extension_names);
	res = vk::createInstance(&inst_info, nullptr, &inst);
	if (res != vk::Result::eSuccess)
	{
		std::cout << "cannot find a compatible Vulkan ICD\n";
		std::cout << "unknown error\n";
		exit(-1);
	}


	/* Make initial call to query gpu_count, then second call for gpu info*/
	uint32_t gpu_count;
	ThrowIfFailed(inst.enumeratePhysicalDevices(&gpu_count, nullptr));
	assert(gpu_count > 0);

	if (gpu_count > 0)
	{
		std::unique_ptr<vk::PhysicalDevice[]> physical_devices(new vk::PhysicalDevice[gpu_count]);
		ThrowIfFailed(inst.enumeratePhysicalDevices(&gpu_count, physical_devices.get()));
		/* For cube demo we just grab the first physical device */
		gpu = physical_devices[0];
	}
	uint32_t queue_family_count = 0;
	gpu.getQueueFamilyProperties(&queue_family_count, nullptr);
	queue_props.reset(new vk::QueueFamilyProperties[queue_family_count]);
	gpu.getQueueFamilyProperties(&queue_family_count, queue_props.get());



}
void VKanRHI::create_device()
{
	float const priorities[1] = { 0.0 };

	vk::DeviceQueueCreateInfo queues[2];
	queues[0].setQueueFamilyIndex(graphics_queue_family_index);
	queues[0].setQueueCount(1);
	queues[0].setPQueuePriorities(priorities);

	auto deviceInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(queues)
		.setEnabledLayerCount(0)
		.setPpEnabledLayerNames(nullptr)
		.setEnabledExtensionCount(enabled_extension_count)
		.setPpEnabledExtensionNames((const char *const *)extension_names)
		.setPEnabledFeatures(nullptr);

	if (separate_present_queue)
	{
		queues[1].setQueueFamilyIndex(present_queue_family_index);
		queues[1].setQueueCount(1);
		queues[1].setPQueuePriorities(priorities);
		deviceInfo.setQueueCreateInfoCount(2);
	}

	ThrowIfFailed( gpu.createDevice(&deviceInfo, nullptr, &device));
	
}

void VKanRHI::InitSwapChain()
{
	auto const createInfo = vk::Win32SurfaceCreateInfoKHR().setHinstance(win32Hinst).setHwnd(win32HWND);
	ThrowIfFailed( inst.createWin32SurfaceKHR(&createInfo, nullptr, &surface));


	uint32_t queue_family_count = 0;
	gpu.getQueueFamilyProperties(&queue_family_count, nullptr);
	std::unique_ptr<vk::Bool32[]> supportsPresent(new vk::Bool32[queue_family_count]);
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		gpu.getSurfaceSupportKHR(i, surface, &supportsPresent[i]);
	}


	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t presentQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			if (graphicsQueueFamilyIndex == UINT32_MAX)
			{
				graphicsQueueFamilyIndex = i;
			}

			if (supportsPresent[i] == 1)
			{
				graphicsQueueFamilyIndex = i;
				presentQueueFamilyIndex = i;
				break;
			}
		}
	}
	if (presentQueueFamilyIndex == UINT32_MAX)
	{
		// If didn't find a queue that supports both graphics and present,
		// then
		// find a separate present queue.
		for (uint32_t i = 0; i < queue_family_count; ++i)
		{
			if (supportsPresent[i] == 1)
			{
				presentQueueFamilyIndex = i;
				break;
			}
		}
	}

	create_device();
	device.getQueue(graphics_queue_family_index, 0, &graphics_queue);
	present_queue = graphics_queue;


	uint32_t formatCount = 0;
	ThrowIfFailed( gpu.getSurfaceFormatsKHR(surface, &formatCount, nullptr));


	std::unique_ptr<vk::SurfaceFormatKHR[]> surfFormats(new vk::SurfaceFormatKHR[formatCount]);
	ThrowIfFailed(gpu.getSurfaceFormatsKHR(surface, &formatCount, surfFormats.get()));
	if (formatCount == 1 && surfFormats[0].format == vk::Format::eUndefined)
	{
		format = vk::Format::eB8G8R8A8Unorm;
	}
	else
	{
		assert(formatCount >= 1);
		format = surfFormats[0].format;
	}



	//----------------------------------------------------------------------------------------------------------------------------

	// Create semaphores to synchronize acquiring presentable buffers before
	// rendering and waiting for drawing to be complete before presenting
	auto const semaphoreCreateInfo = vk::SemaphoreCreateInfo();

	// Create fences that we can use to throttle if we get too far
	// ahead of the image presents
	auto const fence_ci = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);
	for (uint32_t i = 0; i < FRAME_LAG; i++)
	{
		ThrowIfFailed(device.createFence(&fence_ci, nullptr, &fences[i]));
	

		ThrowIfFailed(device.createSemaphore(&semaphoreCreateInfo, nullptr, &image_acquired_semaphores[i]));

		ThrowIfFailed(device.createSemaphore(&semaphoreCreateInfo, nullptr, &draw_complete_semaphores[i]));

		if (separate_present_queue)
		{
			ThrowIfFailed(device.createSemaphore(&semaphoreCreateInfo, nullptr, &image_ownership_semaphores[i]));
		}
	}
	frame_index = 0;

	// Get Memory information and properties
	gpu.getMemoryProperties(&memory_properties);


}
void VKanRHI::Prepare()
{

	auto const cmd_pool_info = vk::CommandPoolCreateInfo().setQueueFamilyIndex(graphics_queue_family_index);
	ThrowIfFailed(device.createCommandPool(&cmd_pool_info, nullptr, &cmd_pool));


	auto const cmd = vk::CommandBufferAllocateInfo()
		.setCommandPool(cmd_pool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	ThrowIfFailed(device.allocateCommandBuffers(&cmd, &this->cmd));


	auto const cmd_buf_info = vk::CommandBufferBeginInfo().setPInheritanceInfo(nullptr);

	ThrowIfFailed(this->cmd.begin(&cmd_buf_info));


}
void VKanRHI::InitContext()
{
	InitDevice();
	InitSwapChain();
	Prepare();
	__debugbreak();
}

void VKanRHI::ThrowIfFailed(vk::Result  r)
{
	if (r != vk::Result::eSuccess)
	{
		__debugbreak();
		std::cout << "Command failed" << std::endl;
	}
}
#endif