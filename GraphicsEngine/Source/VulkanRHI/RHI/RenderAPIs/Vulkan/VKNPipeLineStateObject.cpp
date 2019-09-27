#include "VKNPipeLineStateObject.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/PlatformCore.h"
#include "RHI/RHITypes.h"
#include "VKNBuffers.h"
#include "VKNDeviceContext.h"
#include "VKNRHI.h"
#include "VKNShader.h"
#include "VKNHelpers.h"
#include "Core/Asserts.h"
#include "RHI/RHIRenderPassCache.h"
#include "VKNRenderPass.h"

VKNPipeLineStateObject::VKNPipeLineStateObject(const RHIPipeLineStateDesc & desc, DeviceContext * con) :RHIPipeLineStateObject(desc)
{
	VDevice = (VKNDeviceContext*)con;
}

VKNPipeLineStateObject::~VKNPipeLineStateObject()
{

}

void VKNPipeLineStateObject::Complie()
{
	if (Desc.RenderPass == nullptr)
	{
		if (Desc.RenderPassDesc.TargetBuffer == nullptr)
		{
			Desc.RenderPass = RHIRenderPassCache::Get()->GetOrCreatePass(RHI::GetRenderPassDescForSwapChain());
		}
		else
		{
			Desc.RenderPass = RHIRenderPassCache::Get()->GetOrCreatePass(Desc.RenderPassDesc);
		}
	}
	
	if (Desc.ShaderInUse->IsComputeShader())
	{
		CreateDescriptorSetLayout(true);
		CreatecomputePipelineState();
	}
	else
	{
		CreateDescriptorSetLayout();
		createGraphicsPipeline();
	}
}

void VKNPipeLineStateObject::Release()
{}

void VKNPipeLineStateObject::CreatecomputePipelineState()
{

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = PushRanges.size();
	pipelineLayoutInfo.pPushConstantRanges = PushRanges.data();

	if (vkCreatePipelineLayout(VDevice->device, &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VKNShader* sh = VKNRHI::VKConv(Desc.ShaderInUse->GetShaderProgram());
	VkComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = sh->GetShaderStages()[0];
	pipelineInfo.layout = PipelineLayout;

	if (vkCreateComputePipelines(VDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void VKNPipeLineStateObject::createTextureSampler()
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

	if (vkCreateSampler(VKNRHI::RHIinstance->DevCon->device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}
bool VKNPipeLineStateObject::ParseVertexFormat(std::vector<Shader::VertexElementDESC> desc, std::vector< VkVertexInputAttributeDescription>& attributeDescriptions,
	std::vector< VkVertexInputBindingDescription>& vertexbindings)
{
	int Stride = 0;
	for (int i = 0; i < desc.size(); i++)
	{
		Stride += RHIUtils::GetPixelSize(desc[i].Format);
	}
	//hacky hack
	if (desc[0].Stride != 0)
	{
		Stride = desc[0].Stride;
	}
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = Stride;// sizeof(OGLVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexbindings.push_back(bindingDescription);
	for (int i = 0; i < desc.size(); i++)
	{
		Shader::VertexElementDESC* Element = &desc[i];

		VkVertexInputAttributeDescription Bindingdesc = {};
		Bindingdesc.binding = 0;// Element->InputSlot;
		Bindingdesc.location = i;// Element->InputSlot;
		//covert format
		Bindingdesc.format = VKNHelpers::ConvertFormat(Element->Format);
		Bindingdesc.offset = Element->AlignedByteOffset;
		attributeDescriptions.push_back(Bindingdesc);
	}
	return true;
}

void  VKNPipeLineStateObject::createGraphicsPipeline()
{
	ensure(Desc.RenderPass);
	VKNShader* sh = VKNRHI::VKConv(Desc.ShaderInUse->GetShaderProgram());
	ShaderStages = sh->GetShaderStages();

	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	std::vector< VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector< VkVertexInputBindingDescription> vertexbindings;
	//	Desc.ShaderInUse->GetVertexFormat();


	//RHIDesc.push_back(Shader::VertexElementDESC{ "POSITION", 0, FORMAT_R32G32_FLOAT, 0, 0, Shader::INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
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
	if (Desc.RenderPass->Desc.TargetSwapChain)
	{
		viewport.width = (float)VKNRHI::RHIinstance->swapChainExtent.width;
		viewport.height = (float)VKNRHI::RHIinstance->swapChainExtent.height;
	}
	else
	{
		viewport.width = Desc.RenderPass->Desc.TargetBuffer->GetWidth();
		viewport.height = Desc.RenderPass->Desc.TargetBuffer->GetHeight();
	}

	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	if (Desc.RenderPass->Desc.TargetSwapChain)
	{
		scissor.extent = VKNRHI::RHIinstance->swapChainExtent;
	}
	else
	{
		scissor.extent.width = Desc.RenderPass->Desc.TargetBuffer->GetWidth();
		scissor.extent.height = Desc.RenderPass->Desc.TargetBuffer->GetHeight();
	}

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
	rasterizer.cullMode = Desc.Cull ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments;
	for (int i = 0; i < Desc.RenderPass->Desc.RenderDesc.NumRenderTargets; i++)
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = Desc.Blending;
		if (Desc.Blending)
		{
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}
		BlendAttachments.push_back(colorBlendAttachment);
	}
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = BlendAttachments.size();
	colorBlending.pAttachments = BlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = PushRanges.size();
	pipelineLayoutInfo.pPushConstantRanges = PushRanges.data();

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
	VKNRenderPass* VRP = (VKNRenderPass*)Desc.RenderPass;
	pipelineInfo.renderPass = VRP->RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipelineDepthStencilStateCreateInfo D = {};
	ZeroMemory(&D, sizeof(D));
	D.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	D.depthTestEnable = Desc.DepthStencilState.DepthEnable;
	D.depthWriteEnable = Desc.DepthStencilState.DepthWrite;
	D.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	D.stencilTestEnable = VK_FALSE;
	pipelineInfo.pDepthStencilState = &D;

	if (vkCreateGraphicsPipelines(VDevice->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	//vkDestroyShaderModule(VDevice->device, fragShaderModule, nullptr);
	//vkDestroyShaderModule(VDevice->device, vertShaderModule, nullptr);
}

VkShaderModule VKNPipeLineStateObject::createShaderModule(const std::vector<char>& code)
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

VkShaderModule VKNPipeLineStateObject::createShaderModule(const std::vector<uint32_t>& code)
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

void VKNPipeLineStateObject::CreateDescriptorSetLayout(bool IsCompute )
{
	createTextureSampler();
	std::vector<VkDescriptorSetLayoutBinding> Binds;
	Parms.clear();
	PushRanges.clear();
#if !BASIC_RENDER_ONLY
	Parms = Desc.ShaderInUse->GetShaderParameters();
#else
	Parms.push_back(ShaderParameter(ShaderParamType::CBV, 0, 0));
	Parms.push_back(ShaderParameter(ShaderParamType::SRV, 1, 10));
	Parms.push_back(ShaderParameter(ShaderParamType::CBV, 2, 2));
#endif

	for (int i = 0; i < Parms.size(); i++)
	{
		ShaderParameter* Element = &Parms[i];
		Element->RegisterSpace = 0;
		if (Element->Type == ShaderParamType::CBV)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = VKNShader::GetBindingOffset(Element->Type) + Element->RegisterSlot;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
			Binds.push_back(uboLayoutBinding);
		}
		else if (Element->Type == ShaderParamType::SRV)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = VKNShader::GetBindingOffset(ShaderParamType::SRV) + Element->RegisterSlot;
			samplerLayoutBinding.descriptorCount = Element->NumDescriptors;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			Binds.push_back(samplerLayoutBinding);
		}
		else if (Element->Type == ShaderParamType::Buffer || Element->Type == ShaderParamType::UAV)
		{
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = VKNShader::GetBindingOffset(Element->Type) + Element->RegisterSlot;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
			Binds.push_back(uboLayoutBinding);
		}
		else if (Element->Type == ShaderParamType::RootConstant)
		{
			//todo: pull for shader file
			VkPushConstantRange Range = { };
			Range.offset = 0;
			Range.size = sizeof(float) * 4;
			Range.stageFlags = VK_SHADER_STAGE_ALL;
			PushRanges.push_back(Range);
		}
	}
	if (!IsCompute)
	{
		for (int i = 0; i < 3; i++)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = VKNShader::GetBindingOffset(ShaderParamType::Sampler) + i;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = &textureSampler;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			Binds.push_back(samplerLayoutBinding);
		}
	}
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = Binds.size();
	layoutInfo.pBindings = Binds.data();

	if (vkCreateDescriptorSetLayout(VKNRHI::RHIinstance->DevCon->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

ShaderParameter * VKNPipeLineStateObject::GetRootSigSlot(int id)
{
	for (int i = 0; i < Parms.size(); i++)
	{
		if (Parms[i].SignitureSlot == id)
		{
			return &Parms[i];
		}
	}
	ensure(false);
	return nullptr;
}

