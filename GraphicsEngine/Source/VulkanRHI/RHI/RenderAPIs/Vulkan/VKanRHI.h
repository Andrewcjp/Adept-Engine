#pragma once
#include "EngineGlobals.h"
#include "RHI/RHI.h"

class VkanDeviceContext;
class VkanPipeLineStateObject;
class VKanFramebuffer;
#if BUILD_VULKAN
#define FRAME_LAG 2
#undef NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>
#include "RHI/RHITypes.h"
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
#ifdef NDEBUG
 bool enableValidationLayers = true;
#else
 bool enableValidationLayers = false;
#endif
const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"

};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class VKanRHI : public RHIClass
{
public:
	VKanRHI();
	virtual ~VKanRHI();
	void createFramebuffers();
	VkCommandPool createCommandPool();

	void createSyncObjects();
	void CreateNewObjects();
	void drawFrame();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	void createSwapRT() {};

	static VkanDeviceContext* GetVDefaultDevice();
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

	VkanDeviceContext* DevCon = nullptr;

	VkInstance instance;
	VkDebugUtilsMessengerEXT callback;
	VkSurfaceKHR surface;


	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	RHICommandList* thelist;
	VkCommandPool commandPool;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;
	VkanPipeLineStateObject* SawpPSO;
	VkanPipeLineStateObject* PSO;
	VKanCommandlist* setuplist = nullptr;

	// Inherited via RHIClass

	virtual bool InitWindow(int w, int h) override;
	virtual bool DestoryRHI() override;
	virtual FrameBuffer * CreateFrameBuffer(DeviceContext * Device, const RHIFrameBufferDesc & Desc) override;
	virtual ShaderProgramBase * CreateShaderProgam(DeviceContext * Device = nullptr) override;
	virtual RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) override;
	virtual RHIBuffer * CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext * Device = nullptr) override;
	virtual RHIUAV * CreateUAV(DeviceContext * Device = nullptr) override;
	virtual RHICommandList * CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext * Device = nullptr) override;
	virtual DeviceContext * GetDefaultDevice() override;
	virtual DeviceContext * GetDeviceContext(int index = 0) override;
	virtual void RHISwapBuffers() override;
	virtual void RHIRunFirstFrame() override;
	virtual void ResizeSwapChain(int width, int height) override;
	virtual void WaitForGPU() override;
	virtual void TriggerBackBufferScreenShot() override;
	void cleanup();
	void createInstance();
	static VkInstance * GetInstance();
	void setupDebugCallback();
	void createSurface();
	void createSwapChain();
	void createImageViews();
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR & capabilities);
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	static bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	static std::vector<char> readFile(const std::string & filename);
	void initVulkan();
	class VKanBuffer* buffer = nullptr;
	class VKanRenderPass* Pass = nullptr;
	VKanBuffer* Vertexb = nullptr;
	VKanBuffer* IndexTest = nullptr;
	class VKanTexture* T;
	class VKanShader* Shadertest = nullptr;
	class VKanCommandlist* cmdlist = nullptr;
	RHI_VIRTUAL void SetFullScreenState(bool state) override;
	RHI_VIRTUAL std::string ReportMemory() override;
	RHI_VIRTUAL RHIPipeLineStateObject* CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device) override;
#if ALLOW_RESOURCE_CAPTURE
	RHI_VIRTUAL void TriggerWriteBackResources() override;
#endif

	RHI_VIRTUAL BaseTexture* CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device = nullptr) override;


	RHI_VIRTUAL RHIGPUSyncEvent* CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice) override;


	RHI_VIRTUAL RHIQuery * CreateQuery(EGPUQueryType::Type type, DeviceContext * con) override;


	RHI_VIRTUAL LowLevelAccelerationStructure* CreateLowLevelAccelerationStructure(DeviceContext * Device) override;


	RHI_VIRTUAL HighLevelAccelerationStructure* CreateHighLevelAccelerationStructure(DeviceContext * Device) override;


	RHI_VIRTUAL RHIStateObject* CreateStateObject(DeviceContext* Device) override;
	RHI_VIRTUAL RHIRenderPassDesc GetRenderPassDescForSwapChain(bool ClearScreen = false) override;
	static VKanTexture* VKConv(BaseTexture* T);
	static VKanShader * VKConv(ShaderProgramBase * T);
	static VKanRenderPass * VKConv(RHIRenderPass * T);
	static VKanCommandlist* VKConv(RHICommandList* T);
	static VKanBuffer* VKConv(RHIBuffer* T);
	static VkanDeviceContext* VKConv(DeviceContext* T);
	static VKanFramebuffer* VKConv(FrameBuffer* T);

	Shader_Main* TestShader = nullptr;
	VKanFramebuffer* TestFrameBuffer = nullptr;
	RHI_VIRTUAL RHIRenderPass* CreateRenderPass(RHIRenderPassDesc & Desc, DeviceContext* Device) override;
	VKanCommandlist* PresentList = nullptr;
};
#endif