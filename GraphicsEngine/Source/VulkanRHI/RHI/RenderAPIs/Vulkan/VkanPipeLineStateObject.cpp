#include "VkanPipeLineStateObject.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/PlatformCore.h"
#include "RHI/RHITypes.h"
#include "VkanBuffers.h"
#include "VkanDeviceContext.h"
#include "VKanRHI.h"
#include "VKanShader.h"
#include "VkanHelpers.h"

VkanPipeLineStateObject::VkanPipeLineStateObject(const RHIPipeLineStateDesc & desc, DeviceContext * con) :RHIPipeLineStateObject(desc)
{
	VDevice = (VkanDeviceContext*)con;
}

VkanPipeLineStateObject::~VkanPipeLineStateObject()
{

}

void VkanPipeLineStateObject::Complie()
{
	//CreateRenderPass();
	createGraphicsPipeline();
}

void VkanPipeLineStateObject::Release()
{}

void VkanPipeLineStateObject::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	//samplerInfo.anisotropyEnable = VK_TRUE;
	//samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(VKanRHI::RHIinstance->DevCon->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}
bool VkanPipeLineStateObject::ParseVertexFormat(std::vector<Shader::VertexElementDESC> desc, std::vector< VkVertexInputAttributeDescription>& attributeDescriptions,
	std::vector< VkVertexInputBindingDescription>& vertexbindings)
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(OGLVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexbindings.push_back(bindingDescription);
	for (int i = 0; i < desc.size(); i++)
	{
		Shader::VertexElementDESC* Element = &desc[i];

		VkVertexInputAttributeDescription Bindingdesc = {};
		Bindingdesc.binding = 0;// Element->InputSlot;
		Bindingdesc.location = i;// Element->InputSlot;
		//covert format
		Bindingdesc.format = VkanHelpers::ConvertFormat(Element->Format);
		Bindingdesc.offset = Element->AlignedByteOffset;
		attributeDescriptions.push_back(Bindingdesc);
	}
	return true;
}

void  VkanPipeLineStateObject::createGraphicsPipeline()
{
	CreateTestShader();
	createTextureSampler();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector< VkVertexInputBindingDescription> vertexbindings;
	//	Desc.ShaderInUse->GetVertexFormat();

	std::vector< Shader::VertexElementDESC> RHIDesc;
	RHIDesc.push_back(Shader::VertexElementDESC{ "POSITION", 0, FORMAT_R32G32_FLOAT, 0, 0, Shader::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	RHIDesc = Desc.ShaderInUse->GetVertexFormat();
	ParseVertexFormat(RHIDesc, attributeDescriptions, vertexbindings);

	vertexInputInfo.vertexBindingDescriptionCount = vertexbindings.size();
	vertexInputInfo.pVertexBindingDescriptions = vertexbindings.data();

	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)VKanRHI::RHIinstance->swapChainExtent.width;
	viewport.height = (float)VKanRHI::RHIinstance->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = VKanRHI::RHIinstance->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	CreateDescriptorSetLayout();
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(VDevice->device, &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = ShaderStages.size();
	pipelineInfo.pStages = ShaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = PipelineLayout;
	VKanRenderPass* VRP = (VKanRenderPass*)Desc.RenderPass;
	pipelineInfo.renderPass = VRP->RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(VDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	//vkDestroyShaderModule(VDevice->device, fragShaderModule, nullptr);
	//vkDestroyShaderModule(VDevice->device, vertShaderModule, nullptr);
}

void VkanPipeLineStateObject::CreateTestShader()
{
	std::string root = AssetManager::GetShaderPath() + "VKan\\";
	std::vector<char> vertShaderCode;
	std::vector<char>  fragShaderCode;
	//shader temp
	//vertShaderCode = VKanShader::ComplieShader("VKan\\Tri.vert");
	vertShaderCode = VKanShader::ComplieShader("VKan\\Tri_VS", EShaderType::SHADER_VERTEX, true);
	fragShaderCode = VKanShader::ComplieShader("VKan\\TriHLSL", EShaderType::SHADER_FRAGMENT, true);
	//fragShaderCode = VKanShader::ComplieShader("VKan\\Tri.frag", true);

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	ShaderStages.push_back(fragShaderStageInfo);
	ShaderStages.push_back(vertShaderStageInfo);
}

VkShaderModule VkanPipeLineStateObject::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VDevice->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VkShaderModule VkanPipeLineStateObject::createShaderModule(const std::vector<uint32_t>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VDevice->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void VkanPipeLineStateObject::CreateDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> Binds;
#if 0



#endif
	std::vector<ShaderParameter> Parms;
	Parms.push_back(ShaderParameter(ShaderParamType::CBV, 0, 0));
	Parms.push_back(ShaderParameter(ShaderParamType::SRV, 1, 0));
	Parms.push_back(ShaderParameter(ShaderParamType::CBV, 3, 0));
	for (int i = 0; i < Parms.size(); i++)
	{
		ShaderParameter* Element = &Parms[i];
		if (Element->Type == ShaderParamType::CBV)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = Element->SignitureSlot;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
			Binds.push_back(uboLayoutBinding);
		}
		else if (Element->Type == ShaderParamType::SRV)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = Element->SignitureSlot;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			Binds.push_back(samplerLayoutBinding);
		}
	}
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 2;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = &textureSampler;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		Binds.push_back(samplerLayoutBinding);
	}
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = Binds.size();
	layoutInfo.pBindings = Binds.data();

	if (vkCreateDescriptorSetLayout(VKanRHI::RHIinstance->DevCon->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VKanRenderPass::VKanRenderPass()
{
	VDevice = VKanRHI::RHIinstance->DevCon;
}

void VKanRenderPass::Complie()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VKanRHI::RHIinstance->swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	


	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;





	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(VDevice->device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

}
