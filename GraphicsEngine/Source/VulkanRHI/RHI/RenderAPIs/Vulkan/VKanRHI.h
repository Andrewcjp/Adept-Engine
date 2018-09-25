#pragma once
#include "RHI/RHI.h"
#if BUILD_VULKAN
#define FRAME_LAG 2
#undef NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <include/vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
template<class t>
struct optional
{
	t value()
	{
		return vale;
	}
	optional(t tpye)
	{
		vale = tpye;
	}
	optional()
	{
		vale = 0;
	}
	t vale;
};
struct QueueFamilyIndices
{
	optional<uint32_t> graphicsFamily;
	optional<uint32_t> presentFamily;
	QueueFamilyIndices()
	{

	}
	bool isComplete()
	{
		return true;// graphicsFamily && presentFamily;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
class VKanRHI : public RHIClass
{
public:
	VKanRHI();
	~VKanRHI();
	void createGraphicsPipeline23();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createDescriptorPool();
	void createDescriptorSets();
	void createCommandBuffers();
	void ReadyCmdList(VkCommandBuffer * buffer);
	void CreateDescriptorSet();
	void createSyncObjects();
	void drawFrame();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	void createSwapRT() {};
	void DebugRender();
	virtual bool InitRHI() override;
	HWND	win32HWND;
	static VKanRHI* RHIinstance;
//private:
#if VULKANRHI_EXPORT
	DLLEXPORT static VKanRHI* Get();
#else
	DLLIMPORT static VKanRHI* Get();
#endif
	HINSTANCE win32Hinst;


	VkInstance instance;
	VkDebugUtilsMessengerEXT callback;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	RHICommandList* thelist;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> ListcmdBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;


	VkDescriptorSetLayout descriptorSetLayout;

	// Inherited via RHIClass
	
	virtual bool InitWindow(int w, int h) override;
	virtual bool DestoryRHI() override;
	virtual BaseTexture * CreateTexture(DeviceContext * Device = nullptr) override;
	virtual FrameBuffer * CreateFrameBuffer(DeviceContext * Device, RHIFrameBufferDesc & Desc) override;
	virtual ShaderProgramBase * CreateShaderProgam(DeviceContext * Device = nullptr) override;
	virtual RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) override;
	virtual RHIBuffer * CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext * Device = nullptr) override;
	virtual RHIUAV * CreateUAV(DeviceContext * Device = nullptr) override;
	virtual RHICommandList * CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext * Device = nullptr) override;
	virtual DeviceContext * GetDefaultDevice() override;
	virtual DeviceContext * GetDeviceContext(int index = 0) override;
	virtual void RHISwapBuffers() override;
	virtual void RHIRunFirstFrame() override;
	virtual void ToggleFullScreenState() override;
	virtual void ResizeSwapChain(int width, int height) override;
	virtual void WaitForGPU() override;
	virtual void TriggerBackBufferScreenShot() override;
	void cleanup();
	void createInstance();
	void setupDebugCallback();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	static std::vector<char> readFile(const std::string & filename);
	void initVulkan();
	class VkanDeviceContext* Device = nullptr;
	class VKanBuffer* buffer = nullptr;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	class VKanShader* Shadertest = nullptr;
	class VKanCommandlist* cmdlist = nullptr;
};
#endif