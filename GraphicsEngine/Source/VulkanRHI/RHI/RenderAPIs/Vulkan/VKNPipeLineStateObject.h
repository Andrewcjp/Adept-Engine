#pragma once
#include "VKNRHI.h"
#include "RHI/RHICommandList.h"
#include "RHI/Shader.h"


class VKNPipeLineStateObject :public RHIPipeLineStateObject
{
public:
	VKNPipeLineStateObject(const RHIPipeLineStateDesc& desc, DeviceContext* con);
	~VKNPipeLineStateObject();
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
	VKNDeviceContext* VDevice = nullptr;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
	VkSampler textureSampler;
	std::vector<ShaderParameter> Parms;
	ShaderParameter* GetRootSigSlot(int id);
	std::vector< Shader::VertexElementDESC> RHIDesc;
};
