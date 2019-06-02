#include "VKanRHI.h"
#include "Core/Module/ModuleManager.h"
#include "VKanCommandlist.h"
#include "VKanShader.h"
#include "VKanTexture.h"
#include "VKanFramebuffer.h"
#include "Core/Platform/PlatformCore.h"
#include "VkanBuffers.h"
#include "VkanDeviceContext.h"
#include <fstream>
#include <set>

#include "Core/Platform/Windows/WindowsWindow.h"
#include "VkanPipeLineStateObject.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Core/BaseWindow.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/Mesh/MeshPipelineController.h"
#include "RHI/RHITypes.h"

#if BUILD_VULKAN

VKanRHI* VKanRHI::RHIinstance = nullptr;
static ConsoleVariable ForceNoDebug("ForceNoDebug", 0, ECVarType::LaunchOnly);
VKanRHI::VKanRHI()
{
	enableValidationLayers = true;
	if (ForceNoDebug.GetBoolValue())
	{
		enableValidationLayers = false;
	}

	//win32Hinst = win32inst;
	RHIinstance = this;
}

VKanRHI::~VKanRHI()
{}


void VKanRHI::createGraphicsPipeline23()
{

	createSwapRT();
}


//private:

VKanRHI * VKanRHI::Get()
{
	return RHIinstance;
}


bool VKanRHI::InitWindow(int w, int h)
{
	return false;
}
bool VKanRHI::DestoryRHI()
{
	cleanup();
	return false;
}


BaseTexture* VKanRHI::CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device /*= nullptr*/)
{
	return new VKanTexture();
}

FrameBuffer* VKanRHI::CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc)
{
	//ensureMsgf(Desc.PSO, "Vulkan Needs a PSO to create a framebuffer");
	return new VKanFramebuffer(Device, Desc);
}
void VKanRHI::SetFullScreenState(bool state)
{

}

std::string VKanRHI::ReportMemory()
{
	return "";
}

RHIPipeLineStateObject* VKanRHI::CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device)
{
	return new VkanPipeLineStateObject(Desc, Device);
}

RHIGPUSyncEvent* VKanRHI::CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice)
{
	return nullptr;
}

RHIQuery * VKanRHI::CreateQuery(EGPUQueryType::Type type, DeviceContext * con)
{
	throw std::logic_error("The method or operation is not implemented.");
}

LowLevelAccelerationStructure* VKanRHI::CreateLowLevelAccelerationStructure(DeviceContext * Device)
{
	throw std::logic_error("The method or operation is not implemented.");
}

HighLevelAccelerationStructure* VKanRHI::CreateHighLevelAccelerationStructure(DeviceContext * Device)
{
	throw std::logic_error("The method or operation is not implemented.");
}

RHIStateObject* VKanRHI::CreateStateObject(DeviceContext* Device)
{
	throw std::logic_error("The method or operation is not implemented.");
}

VKanTexture * VKanRHI::VKConv(BaseTexture * T)
{
	return static_cast<VKanTexture*>(T);
}

VKanCommandlist * VKanRHI::VKConv(RHICommandList * T)
{
	return static_cast<VKanCommandlist*>(T);
}

VKanBuffer * VKanRHI::VKConv(RHIBuffer * T)
{
	return static_cast<VKanBuffer*>(T);
}

VkanDeviceContext * VKanRHI::VKConv(DeviceContext * T)
{
	return static_cast<VkanDeviceContext*>(T);
}

VKanFramebuffer * VKanRHI::VKConv(FrameBuffer * T)
{
	return static_cast<VKanFramebuffer*>(T);
}

#if ALLOW_RESOURCE_CAPTURE
void VKanRHI::TriggerWriteBackResources()
{

}
#endif
ShaderProgramBase * VKanRHI::CreateShaderProgam(DeviceContext * Device/* = nullptr*/)
{
	return new VKanShader();
}
RHITextureArray * VKanRHI::CreateTextureArray(DeviceContext * Device, int Length)
{
	return new VkanTextureArray(Device, Length);
}
RHIBuffer * VKanRHI::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext * Device)
{
	return new VKanBuffer(type, Device);
}
RHIUAV * VKanRHI::CreateUAV(DeviceContext * Device)
{
	return new VkanUAV();
}
RHICommandList * VKanRHI::CreateCommandList(ECommandListType::Type Type/* = ECommandListType::Graphics*/, DeviceContext * Device/* = nullptr*/)
{
	return new VKanCommandlist(Type, Device);
}
DeviceContext * VKanRHI::GetDefaultDevice()
{
	return DevCon;
}

VkanDeviceContext* VKanRHI::GetVDefaultDevice()
{
	return RHIinstance->DevCon;
}

DeviceContext * VKanRHI::GetDeviceContext(int index)
{
	if (index != 0)
	{
		return nullptr;
	}
	return DevCon;
}

void VKanRHI::RHISwapBuffers()
{
	if (RHI::GetFrameCount() == 0)
	{
		RHIRunFirstFrame();
	}
	drawFrame();
}

void VKanRHI::RHIRunFirstFrame()
{
	//setuplist->Execute();
}

void VKanRHI::ResizeSwapChain(int width, int height)
{}
void VKanRHI::WaitForGPU()
{}
void VKanRHI::TriggerBackBufferScreenShot()
{}



class VulkanRHIModule : public RHIModule
{
	virtual RHIClass* GetRHIClass()
	{
		return new VKanRHI();
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


void VKanRHI::cleanup()
{
	vkDeviceWaitIdle(DevCon->device);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, callback, nullptr);
	}
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(DevCon->device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(DevCon->device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(DevCon->device, inFlightFences[i], nullptr);
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

void  VKanRHI::createInstance()
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

	DevCon = new VkanDeviceContext();
	DevCon->Init();
}
VkInstance* VKanRHI::GetInstance()
{
	return &RHIinstance->instance;
}


void  VKanRHI::createSurface()
{
	/*if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}*/

}




void  VKanRHI::createSwapChain()
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

	QueueFamilyIndices indices = VkanDeviceContext::findQueueFamilies(DevCon->physicalDevice);
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

void  VKanRHI::createImageViews()
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


void  VKanRHI::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = Pass->RenderPass;
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

VkCommandPool  VKanRHI::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = VkanDeviceContext::findQueueFamilies(DevCon->physicalDevice);

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

void  VKanRHI::createSyncObjects()
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
void VKanRHI::CreateNewObjects()
{

	Pass = new VKanRenderPass();
	Pass->Complie();
	TestShader = new Shader_Main(true);
	RHIPipeLineStateDesc DEsc;
	DEsc.RenderPass = Pass;
	DEsc.ShaderInUse = TestShader;
	PSO = new VkanPipeLineStateObject(DEsc, DevCon);
	PSO->Complie();
	Vertexb = new VKanBuffer(ERHIBufferType::Vertex, nullptr);
#if 0
	glm::vec2 positions[3] = {
		glm::vec2(0.0, -0.5),
		glm::vec2(0.5, 0.7),
		glm::vec2(-0.5, 0.5)
	};
	Vertexb->CreateVertexBuffer(sizeof(glm::vec2), sizeof(positions));
#else
	OGLVertex positions[3]{
		OGLVertex(),
		OGLVertex(),
		OGLVertex()
	};
	positions[0].m_position = glm::vec3(0.0, -0.5, 1.0f);
	positions[1].m_position = glm::vec3(0.5, 0.7, 1.0f);
	positions[2].m_position = glm::vec3(-0.5, 0.5, 1.0f);

	positions[0].m_texcoords = glm::vec3(0.0, -0.5, 0.0f);
	positions[1].m_texcoords = glm::vec3(0.5, 0.7, 0.0f);
	positions[2].m_texcoords = glm::vec3(-0.5, 0.5, 0.0f);
	Vertexb->CreateVertexBuffer(sizeof(OGLVertex), sizeof(positions));
#endif

	Vertexb->UpdateVertexBuffer(&positions, sizeof(positions));

	IndexTest = new VKanBuffer(ERHIBufferType::Index, nullptr);
	short  ind[3]{ 1,2,0 };
	IndexTest->CreateIndexBuffer(sizeof(short), sizeof(ind));
	IndexTest->UpdateVertexBuffer(&ind, sizeof(ind));
}

void  VKanRHI::drawFrame()
{

	vkWaitForFences(DevCon->device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(DevCon->device, 1, &inFlightFences[currentFrame]);
	vkDeviceWaitIdle(DevCon->device);
	uint32_t imageIndex;
	vkAcquireNextImageKHR(DevCon->device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	if (cmdlist == nullptr)
	{
		cmdlist = setuplist;//new VKanCommandlist(ECommandListType::Graphics, DevCon);
	}
	else
	{
		cmdlist->ResetList();
	}

	RHIRenderPassDesc Info = RHIRenderPassDesc();
	cmdlist->BeginRenderPass(Info);
	cmdlist->SetPipelineStateObject(PSO);
	cmdlist->SetConstantBufferView(buffer, 0, 0);
	cmdlist->SetTexture(T, 1);
	BaseWindow::GetCurrentRenderer()->SceneRender->BindMvBuffer(cmdlist, 2);
	BaseWindow::GetCurrentRenderer()->SceneRender->Controller->RenderPass(ERenderPass::DepthOnly, cmdlist);

#if 1
	cmdlist->SetVertexBuffer(Vertexb);
	cmdlist->SetIndexBuffer(IndexTest);
	cmdlist->DrawIndexedPrimitive(3, 1, 0, 0, 0);
#endif
	cmdlist->EndRenderPass();
	cmdlist->Execute();

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = cmdlist->GetCommandBuffer();
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(DevCon->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
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


VkSurfaceFormatKHR VKanRHI::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR VKanRHI::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
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

VkExtent2D  VKanRHI::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

SwapChainSupportDetails  VKanRHI::querySwapChainSupport(VkPhysicalDevice device)
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


bool  VKanRHI::checkDeviceExtensionSupport(VkPhysicalDevice device)
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


std::vector<const char*>  VKanRHI::getRequiredExtensions()
{

	std::vector<const char*> extensions;
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

bool VKanRHI::checkValidationLayerSupport()
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

std::vector<char> VKanRHI::readFile(const std::string& filename)
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
	//std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
	Log::LogMessage("validation: " + std::string(pCallbackData->pMessage));
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		__debugbreak();
	}
	return false;
}


void VKanRHI::initVulkan()
{
	createInstance();
	setupDebugCallback();
	createSurface();
	setuplist = new VKanCommandlist(ECommandListType::Compute, RHI::GetDefaultDevice());
	setuplist->ResetList();
	//pickPhysicalDevice();
	//createLogicalDevice();
	createSwapChain();
	createImageViews();

	CreateNewObjects();

	//	createGraphicsPipeline();
	createFramebuffers();
	commandPool = createCommandPool();
	buffer = new VKanBuffer(ERHIBufferType::Constant, nullptr);
	glm::vec4 data = glm::vec4(1, 0.2, 0.8, 1);
	buffer->CreateConstantBuffer(sizeof(data), 1);
	buffer->UpdateConstantBuffer(glm::value_ptr(data), 0);
	T = new VKanTexture();
	T->CreateFromFile(AssetPathRef("texture\\ammoc03.jpg"));

	createSyncObjects();

}

void  VKanRHI::setupDebugCallback()
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


bool VKanRHI::InitRHI()
{
	win32Hinst = PlatformWindow::GetHInstance();
	win32HWND = PlatformWindow::GetHWND();
	initVulkan();
	return true;
}


#endif