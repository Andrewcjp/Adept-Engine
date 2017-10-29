#pragma once

#include "../RHI/ShaderBase.h"
class ShaderProgramBase
{

protected:
	ShaderProgramHandle			m_systemHandle; //handle assigned by opengl;
	ShaderBase*					m_shaders[MAX_NUM_SHADER_PER_PROGRAM];
	int							m_shaderCount;

public:
	ShaderProgramBase()
	{
		m_systemHandle = INVALID_SHADER_PROGRAM_HANDLE;
		for (int i = 0; i < MAX_NUM_SHADER_PER_PROGRAM; i++)
		{
			m_shaders[i] = NULL;
		}
		m_shaderCount = 0;
	}
	virtual						~ShaderProgramBase();

	inline ShaderProgramHandle	GetProgramHandle()
	{
		return m_systemHandle;
	}

	virtual void						CreateShaderProgram() = 0;
	//void						AttachShader(ShaderBase* shader);
	virtual EShaderError						AttachAndCompileShaderFromFile(const char * filename, EShaderType type) = 0;
	virtual void						BuildShaderProgram() = 0;
	virtual void						DeleteShaderProgram() = 0;
	virtual void						ActivateShaderProgram() = 0;
	virtual void						DeactivateShaderProgram() = 0;
	virtual void						SetUniform1UInt(unsigned int value, const char* param) = 0;
	virtual void						SetAttrib4Float(float f1, float f2, float f3, float f4, const char* param) = 0;
	virtual void						BindAttributeLocation(int index, const char* param_name) = 0;
};

