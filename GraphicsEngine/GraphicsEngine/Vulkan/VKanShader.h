#pragma once
#include "../RHI/ShaderProgramBase.h"
class VKanShader : public ShaderProgramBase
{
public:
	VKanShader();
	~VKanShader();

	// Inherited via ShaderProgramBase
	virtual void CreateShaderProgram() override;
	virtual EShaderError AttachAndCompileShaderFromFile(const char * filename, EShaderType type) override;
	virtual void BuildShaderProgram() override;
	virtual void DeleteShaderProgram() override;
	virtual void ActivateShaderProgram() override;
	virtual void DeactivateShaderProgram() override;
};

