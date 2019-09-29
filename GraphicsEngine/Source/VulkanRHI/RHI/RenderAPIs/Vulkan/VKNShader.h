#pragma once
#if BUILD_VULKAN
#include "RHI/ShaderProgramBase.h"
#include "VKNRHI.h"
#include "Vulkan/glslang/Public/ShaderLang.h"
struct ComplieInfo
{
	EShLanguage stage;
	bool HLSL = false;
};

class VKNShader : public ShaderProgramBase
{
public:
	VKNShader();
	~VKNShader();

	// Inherited via ShaderProgramBase
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint) override;
	static std::vector<char> readFile(const std::string & filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	 bool GenerateSpirv(const std::string Source, ComplieInfo & CompilerInfo, std::string & OutErrors, std::vector<char>& OutSpirv, std::string name);
	 std::vector<char> ComplieShader(std::string name, EShaderType::Type T, bool HLSL = false);

	std::vector<char> ComplieShader_Local(std::string name, EShaderType::Type T, bool HLSL, std::string ShaderDebugName);
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
	std::string entyPoint;
};

#endif