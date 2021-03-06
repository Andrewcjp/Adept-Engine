#include "VKNRHI.h"
#include "Core/Module/ModuleManager.h"
#include "VKNCommandlist.h"
#include "VKNShader.h"
#include "VKNTexture.h"
#include "VKNFramebuffer.h"
#include "Core/Platform/PlatformCore.h"
#include "VKNBuffers.h"
#include "VKNDeviceContext.h"
#include <fstream>
#include <set>

#include "Core/Platform/Windows/WindowsWindow.h"
#include "VKNPipeLineStateObject.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Core/BaseWindow.h"

#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/Mesh/MeshPipelineController.h"
#include "RHI/RHITypes.h"
#include "RHI/RHIRenderPassCache.h"
#include "Rendering/Core/RenderingUtils.h"
#include "VKNRenderPass.h"
#include "Rendering/Core/Defaults.h"
#include "VKNDescriptorPool.h"



VKNRHI* VKNRHI::RHIinstance = nullptr;
static ConsoleVariable ForceNoDebug("ForceNoDebug", 0, ECVarType::LaunchOnly);
VKNRHI::VKNRHI()
{
#if _DEBUG
	enableValidationLayers = true;
	if (ForceNoDebug.GetBoolValue())
	{
		enableValidationLayers = false;
	}
#else
	enableValidationLayers = false;
#endif

	//win32Hinst = win32inst;
	RHIinstance = this;
}

VKNRHI::~VKNRHI()
{}


VKNRHI * VKNRHI::Get()
{
	return RHIinstance;
}

bool VKNRHI::InitWindow(int w, int h)
{
	win32Hinst = PlatformWindow::GetHInstance();
	win32HWND = PlatformWindow::GetHWND();

	//pickPhysicalDevice();
	//createLogicalDevice();
	createSwapChain();
	createImageViews();

	CreateNewObjects();
	createFramebuffers();
	commandPool = createCommandPool();
	buffer = new VKNBuffer(ERHIBufferType::Constant, nullptr);
	glm::vec4 data = glm::vec4(1, 0.2, 0.8, 1);
	buffer->CreateConstantBuffer(sizeof(data), 1);
	buffer->UpdateConstantBuffer(glm::value_ptr(data), 0);
	createSyncObjects();
	return false;
}

bool VKNRHI::DestoryRHI()
{
	cleanup();
	return false;
}

BaseTexture* VKNRHI::CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device /*= nullptr*/)
{
	return new VKNTexture();
}

FrameBuffer* VKNRHI::CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc)
{
	return new VKNFramebuffer(Device, Desc);
}
void VKNRHI::SetFullScreenState(bool state)
{

}

std::string VKNRHI::ReportMemory()
{
	return "";
}

RHIPipeLineStateObject* VKNRHI::CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device)
{
	return new VKNPipeLineStateObject(Desc, Device);
}

RHIGPUSyncEvent* VKNRHI::CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice)
{
	return nullptr;
}

RHIQuery * VKNRHI::CreateQuery(EGPUQueryType::Type type, DeviceContext * con)
{
	throw std::logic_error("The method or operation is not implemented.");
}

RHIRenderPassDesc VKNRHI::GetRenderPassDescForSwapChain(bool ClearScreen)
{
	RHIRenderPassDesc desc = RHIClass::GetRenderPassDescForSwapChain(ClearScreen);
	desc.RenderDesc.NumRenderTargets = 1;
	desc.RenderDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_B8G8R8A8_UNORM;
	desc.StoreOp = ERenderPassStoreOp::Store;
	if (ClearScreen)
	{
		desc.InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_UNDEFINED;
	}
	else
	{
		desc.InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PRESENT;
	}

	desc.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PRESENT;
	desc.TargetSwapChain = true;
	return desc;
}

VKNTexture * VKNRHI::VKConv(BaseTexture * T)
{
	return static_cast<VKNTexture*>(T);
}

VKNShader * VKNRHI::VKConv(ShaderProgramBase * T)
{
	return static_cast<VKNShader*>(T);
}

VKNRenderPass * VKNRHI::VKConv(RHIRenderPass * T)
{
	return static_cast<VKNRenderPass*>(T);
}

VKNCommandlist * VKNRHI::VKConv(RHICommandList * T)
{
	return static_cast<VKNCommandlist*>(T);
}

VKNBuffer * VKNRHI::VKConv(RHIBuffer * T)
{
	return static_cast<VKNBuffer*>(T);
}

VKNDeviceContext * VKNRHI::VKConv(DeviceContext * T)
{
	return static_cast<VKNDeviceContext*>(T);
}

VKNFramebuffer * VKNRHI::VKConv(FrameBuffer * T)
{
	return static_cast<VKNFramebuffer*>(T);
}

VKNPipeLineStateObject * VKNRHI::VKConv(RHIPipeLineStateObject * T)
{
	return static_cast<VKNPipeLineStateObject*>(T);
}

RHIRenderPass* VKNRHI::CreateRenderPass(const RHIRenderPassDesc & Desc, DeviceContext* Device)
{
	return new VKNRenderPass(Desc, Device);
}

RHIInterGPUStagingResource* VKNRHI::CreateInterGPUStagingResource(DeviceContext* Owner, const InterGPUDesc& desc)
{
	ensureFatalMsgf(false, "Vulkan does not support UnLinked MGPU so an inter-GPU staging resource cannot be created");
	return nullptr;
}

#if ALLOW_RESOURCE_CAPTURE
void VKNRHI::TriggerWriteBackResources()
{

}
#endif
ShaderProgramBase * VKNRHI::CreateShaderProgam(DeviceContext * Device/* = nullptr*/)
{
	return new VKNShader();
}
RHITextureArray * VKNRHI::CreateTextureArray(DeviceContext * Device, int Length)
{
	return new VkanTextureArray(Device, Length);
}
RHIBuffer * VKNRHI::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext * Device)
{
	return new VKNBuffer(type, Device);
}
RHIUAV * VKNRHI::CreateUAV(DeviceContext * Device)
{
	return new VknUAV();
}
RHICommandList * VKNRHI::CreateCommandList(ECommandListType::Type Type/* = ECommandListType::Graphics*/, DeviceContext * Device/* = nullptr*/)
{
	return new VKNCommandlist(Type, Device);
}
DeviceContext * VKNRHI::GetDefaultDevice()
{
	return DevCon;
}

VKNDeviceContext* VKNRHI::GetVDefaultDevice()
{
	return RHIinstance->DevCon;
}

DeviceContext * VKNRHI::GetDeviceContext(int index)
{
	if (index != 0)
	{
		return nullptr;
	}
	return DevCon;
}

void VKNRHI::RHISwapBuffers()
{
	drawFrame();
}

void VKNRHI::RHIRunFirstFrame()
{
#if !BASIC_RENDER_ONLY
	setuplist->Execute();
	vkDeviceWaitIdle(DevCon->device);
#endif
}

void VKNRHI::ResizeSwapChain(int width, int height)
{}
void VKNRHI::WaitForGPU()
{}
void VKNRHI::TriggerBackBufferScreenShot()
{}



class VulkanRHIModule : public RHIModule
{
	virtual RHIClass* GetRHIClass()
	{
		return new VKNRHI();
	}
};

#ifdef VULKANRHI_EXPORT
IMPLEMENT_MODULE_DYNAMIC(VulkanRHIModule);
#endif
const int WIDTH = 800;
const int HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}


void VKNRHI::cleanup()
{
	vkDeviceWaitIdle(DevCon->device);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
	}
	if (renderFinishedSemaphores.size() > 0)
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(DevCon->device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(DevCon->device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(DevCon->device, inFlightFences[i], nullptr);
		}
	}
	vkDestroyCommandPool(DevCon->device, commandPool, nullptr);

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(DevCon->device, framebuffer, nullptr);
	}

	//	vkDestroyPipeline(DevCon->device, graphicsPipeline, nullptr);
	//	vkDestroyPipelineLayout(DevCon->device, pipelineLayout, nullptr);
	//	vkDestroyRenderPass(DevCon->device, renderPass, nullptr);
	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(DevCon->device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(DevCon->device, swapChain, nullptr);
	vkDestroyDevice(DevCon->device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void  VKNRHI::createInstance()
{

	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
	auto win32createInfo = VkWin32SurfaceCreateInfoKHR();
	win32createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32createInfo.hinstance = (win32Hinst);
	win32createInfo.hwnd = (win32HWND);
	/*(inst.createWin32SurfaceKHR(&createInfo, nullptr, &surface));*/
	auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	VkResult r = CreateWin32SurfaceKHR(instance, &win32createInfo, nullptr, &surface);
	if (!CreateWin32SurfaceKHR || r != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	DevCon = new VKNDeviceContext();
	DevCon->Init();
}
VkInstance* VKNRHI::GetInstance()
{
	return &RHIinstance->instance;
}


void  VKNRHI::createSurface()
{
	/*if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}*/

}




void  VKNRHI::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(DevCon->physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	//change for more buffering!
	createInfo.minImageCount = 2;//double buffering the data
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = VKNDeviceContext::findQueueFamilies(DevCon->physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (/*indices.graphicsFamily != indices.presentFamily*/false)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(DevCon->device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(DevCon->device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(DevCon->device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void  VKNRHI::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(DevCon->device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

#include "Core/Assets/AssetManager.h"


void  VKNRHI::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(VKNRHI::VKNRHI::GetRenderPassDescForSwapChain()))->RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(DevCon->device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

VkCommandPool  VKNRHI::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = VKNDeviceContext::findQueueFamilies(DevCon->physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPool cmdpool;
	if (vkCreateCommandPool(DevCon->device, &poolInfo, nullptr, &cmdpool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
	return cmdpool;
}

void  VKNRHI::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(DevCon->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(DevCon->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(DevCon->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
void VKNRHI::CreateNewObjects()
{
	RHIRenderPassCache::Get()->GetOrCreatePass(VKNRHI::GetRenderPassDescForSwapChain());
	PresentList = new VKNCommandlist(ECommandListType::Graphics, RHI::GetDefaultDevice());
}

void  VKNRHI::drawFrame()
{

	vkWaitForFences(DevCon->device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(DevCon->device, 1, &inFlightFences[currentFrame]);
	vkDeviceWaitIdle(DevCon->device);
	uint32_t imageIndex;
	vkAcquireNextImageKHR(DevCon->device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	VKNRHI::VKConv(RHI::GetDefaultDevice())->pool->ResetAllocations();
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	PresentList = setuplist;
#if 1
	PresentList->ResetList();
	//PresentList->Execute();
	vkEndCommandBuffer(*PresentList->GetCommandBuffer());
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = PresentList->GetCommandBuffer();
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(DevCon->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
#endif
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(DevCon->presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	vkDeviceWaitIdle(DevCon->device);
	//Sleep(100);
}


VkSurfaceFormatKHR VKNRHI::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VKNRHI::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D  VKNRHI::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { WIDTH, HEIGHT };

		actualExtent.width = glm::max(capabilities.minImageExtent.width, glm::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = glm::max(capabilities.minImageExtent.height, glm::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

SwapChainSupportDetails  VKNRHI::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, RHIinstance->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, RHIinstance->surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, RHIinstance->surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, RHIinstance->surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, RHIinstance->surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}


bool  VKNRHI::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


std::vector<const char*>  VKNRHI::getRequiredExtensions()
{

	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		//	extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}

	return extensions;
}

bool VKNRHI::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

std::vector<char> VKNRHI::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		return false;
	}
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		return false;
	}
	std::string msg = std::string(pCallbackData->pMessage);
	Log::LogMessage("validation: " + msg);
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		if (msg.find("Type mismatch") == -1)
		{
			__debugbreak();
		}
	}
	return false;
}


void VKNRHI::initVulkan()
{
	createInstance();
	setupDebugCallback();
	createSurface();
	setuplist = new VKNCommandlist(ECommandListType::Compute, RHI::GetDefaultDevice());
	setuplist->ResetList();

}

void  VKNRHI::setupDebugCallback()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
}


bool VKNRHI::InitRHI()
{

	initVulkan();
	return true;
}
