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
	//virtual void CreateShaderProgram() override;
	//virtual EShaderError AttachAndCompileShaderFromFile(const char * filename, EShaderType type) override;
	//virtual void BuildShaderProgram() override;
	//virtual void DeleteShaderProgram() override;
	//virtual void ActivateShaderProgram() override;
	//virtual void DeactivateShaderProgram() override;

	// Inherited via ShaderProgramBase
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type) override;
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint) override;
	static std::vector<char> readFile(const std::string & filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void CreateTestShader();
	void SetupPSO();
	void CreateRenderPass() {};
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkExtent2D swapChainExtent;
};

#endif