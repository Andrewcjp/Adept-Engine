#include "VKNDeviceContext.h"
#include "VKNTimeManager.h"
#include <set>
#include "VKNDescriptorPool.h"
#include "VKNRHI.h"
#include "VKNCommandlist.h"



VKNDeviceContext::VKNDeviceContext()
{
	TimeManager = new VKNTimeManager();

}


VKNDeviceContext::~VKNDeviceContext()
{}

void VKNDeviceContext::ResetDeviceAtEndOfFrame()
{}

void VKNDeviceContext::SampleVideoMemoryInfo()
{}

std::string VKNDeviceContext::GetMemoryReport()
{
	return std::string();
}

void VKNDeviceContext::DestoryDevice()
{}

void VKNDeviceContext::WaitForGpu()
{
	vkDeviceWaitIdle(device);
}

void VKNDeviceContext::WaitForCopy()
{}

void VKNDeviceContext::ResetSharingCopyList()
{}

void VKNDeviceContext::NotifyWorkForCopyEngine()
{}

void VKNDeviceContext::UpdateCopyEngine()
{
	VKNRHI::RHIinstance->setuplist->Execute();
	vkDeviceWaitIdle(device);
	VKNRHI::RHIinstance->setuplist->ResetList();
}

void VKNDeviceContext::ResetCopyEngine()
{}

void VKNDeviceContext::GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{}

void VKNDeviceContext::CPUWaitForAll()
{}

void VKNDeviceContext::InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{}

RHITimeManager * VKNDeviceContext::GetTimeManager()
{
	return TimeManager;
}

bool  VKNDeviceContext::isDeviceSuitable(VkPhysicalDevice pdevice)
{
	QueueFamilyIndices indices = findQueueFamilies(pdevice);

	bool extensionsSupported = VKNRHI::checkDeviceExtensionSupport(pdevice);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = VKNRHI::querySwapChainSupport(pdevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}
void  VKNDeviceContext::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*VKNRHI::GetInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(*VKNRHI::GetInstance(), &deviceCount, devices.data());

	for (const auto& idevice : devices)
	{
		if (isDeviceSuitable(idevice))
		{
			physicalDevice = idevice;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void  VKNDeviceContext::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.geometryShader = true;
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VKNDeviceContext::Init()
{
	pickPhysicalDevice();
	createLogicalDevice();
	pool = new VKNDescriptorPool(this);
}

QueueFamilyIndices  VKNDeviceContext::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VKNRHI::RHIinstance->surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}