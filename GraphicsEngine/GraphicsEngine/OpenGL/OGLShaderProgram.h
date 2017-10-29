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
	virtual void						SetUniform1UInt(unsigned int value, const char* param) override;;
	virtual void						SetAttrib4Float(float f1, float f2, float f3, float f4, const char* param) override;;
	virtual void						BindAttributeLocation(int index, const char* param_name) override;;

};

