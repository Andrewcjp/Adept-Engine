#pragma once
#include "OpenGL\OGLShader.h"
class OGLShaderProgram : public ShaderProgramBase
{

private:
	

public:
	OGLShaderProgram() :ShaderProgramBase()
	{
		
	}
	virtual						~OGLShaderProgram();


	virtual void						CreateShaderProgram() override;
	virtual void						AttachShader(ShaderBase* shader);
	EShaderError						AttachAndCompileShaderFromFile(const char* filename, EShaderType type) override;
	virtual void						BuildShaderProgram() override;;
	virtual void						DeleteShaderProgram() override;;
	virtual void						ActivateShaderProgram() override;;
	virtual void						DeactivateShaderProgram() override;;

};

