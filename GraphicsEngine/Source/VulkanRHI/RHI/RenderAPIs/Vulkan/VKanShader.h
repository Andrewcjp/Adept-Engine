#pragma once
#if BUILD_VULKAN
#include "RHI/ShaderProgramBase.h"
#include "VKanRHI.h"
#include "Vulkan/glslang/Public/ShaderLang.h"


class VKanShader : public ShaderProgramBase
{
public:
	VKanShader();
	~VKanShader();

	// Inherited via ShaderProgramBase
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint) override;
	static std::vector<char> readFile(const std::string & filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> ComplieShader(std::string name, EShaderType::Type T, bool HLSL = false);

	std::vector<char> ComplieShader_Local(std::string name, EShaderType::Type T, bool HLSL);
	std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages();
	static EShLanguage GetStage(EShaderType::Type T);

	VkShaderStageFlagBits ConvStage(EShaderType::Type T);

	void CreateRenderPass() {};
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkExtent2D swapChainExtent;
	static int GetBindingOffset(ShaderParamType::Type Type);
private:
	std::vector<VkPipelineShaderStageCreateInfo> Stages;
};

#endif