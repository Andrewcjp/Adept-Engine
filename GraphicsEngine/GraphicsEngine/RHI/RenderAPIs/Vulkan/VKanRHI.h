#pragma once
#define FRAME_LAG 2
#undef NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <include/vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
class VKanRHI
{
public:
	VKanRHI(HINSTANCE win32inst);
	~VKanRHI();
	void InitDevice();
	void create_device();
	void InitSwapChain();
	void Prepare();
	void InitContext();
	HWND	win32HWND;
private:
	vk::Image image;
	vk::CommandBuffer cmd;
	vk::CommandBuffer graphics_to_present_cmd;
	vk::ImageView view;
	vk::Buffer uniform_buffer;
	vk::DeviceMemory uniform_memory;
	vk::Framebuffer framebuffer;
	vk::DescriptorSet descriptor_set;

	HINSTANCE win32Hinst;
	

	///core rhi

	vk::Instance inst;
	vk::PhysicalDevice gpu;
	vk::Device device;

	vk::SurfaceKHR surface;

	vk::Queue graphics_queue;
	vk::Queue present_queue;
	uint32_t graphics_queue_family_index;
	uint32_t present_queue_family_index;
	vk::Semaphore image_acquired_semaphores[FRAME_LAG];
	vk::Semaphore draw_complete_semaphores[FRAME_LAG];
	vk::Semaphore image_ownership_semaphores[FRAME_LAG];
	vk::PhysicalDeviceProperties gpu_props;
	std::unique_ptr<vk::QueueFamilyProperties[]> queue_props;
	vk::PhysicalDeviceMemoryProperties memory_properties;

	vk::PresentModeKHR presentMode;
	vk::Fence fences[FRAME_LAG];
	uint32_t frame_index;


	uint32_t enabled_extension_count;
	char const *extension_names[64];
	bool separate_present_queue = false;
	vk::CommandPool cmd_pool;
	vk::CommandPool present_cmd_pool;


	vk::Format format;
	vk::ColorSpaceKHR color_space;
	void ThrowIfFailed(vk::Result  r);
};
