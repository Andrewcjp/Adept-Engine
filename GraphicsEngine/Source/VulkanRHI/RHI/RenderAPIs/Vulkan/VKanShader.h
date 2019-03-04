#pragma once
#if BUILD_VULKAN
#include "RHI/ShaderProgramBase.h"
#include "VKanRHI.h"


class VKanShader : public ShaderProgramBase
{
public:
	VKanShader();
	~VKanShader();

	// Inherited via ShaderProgramBase
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type) override;
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint) override;
	static std::vector<char> readFile(const std::string & filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	static std::vector<char> ComplieShader(std::string name, bool frag = false, bool HLSL = false);
	void CreateTestShader();
	void SetupPSO();
	void CreateRenderPass() {};
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkExtent2D swapChainExtent;
private:

};

#endif