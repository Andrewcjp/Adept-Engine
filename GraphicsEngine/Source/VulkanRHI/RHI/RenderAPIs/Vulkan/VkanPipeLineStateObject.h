#pragma once
#include "RHI/RHICommandList.h"
#include "RHI/Shader.h"
#include "VKanRHI.h"
#if BUILD_VULKAN
class VkanPipeLineStateObject :public RHIPipeLineStateObject
{
public:
	VkanPipeLineStateObject(const RHIPipeLineStateDesc& desc, DeviceContext* con);
	~VkanPipeLineStateObject();
	virtual void Complie() override;
	virtual void Release() override;
	void createTextureSampler();
	bool ParseVertexFormat(std::vector<Shader::VertexElementDESC> desc, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, std::vector<VkVertexInputBindingDescription>& vertexbindings);
	void createGraphicsPipeline();

	void CreateTestShader();

	//Needs to contain Render pass and pipeline
	VkShaderModule createShaderModule(const std::vector<char>& code);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
	void CreateDescriptorSetLayout();
	//private:
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
	VkanDeviceContext* VDevice = nullptr;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
	VkSampler textureSampler;
	std::vector<ShaderParameter> Parms;
	ShaderParameter* GetRootSigSlot(int id);
};


class VKanRenderPass :public RHIRenderPass
{
public:
	VKanRenderPass(RHIRenderPassDesc & desc, DeviceContext* Device);
	virtual void Complie() override;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	VkanDeviceContext* VDevice = nullptr;
};
#endif