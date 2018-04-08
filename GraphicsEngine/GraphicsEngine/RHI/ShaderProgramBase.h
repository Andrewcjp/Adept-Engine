#pragma once

#include "../RHI/ShaderBase.h"
#include <vector>
class ShaderProgramBase
{

protected:
	ShaderProgramHandle			m_systemHandle; //handle assigned by opengl;
	ShaderBase*					m_shaders[MAX_NUM_SHADER_PER_PROGRAM];
	int							m_shaderCount;
	bool						IsCompute = false;
public:
	ShaderProgramBase()
	{}
	virtual								~ShaderProgramBase();

	inline ShaderProgramHandle	GetProgramHandle()
	{
		return m_systemHandle;
	}

	virtual void						CreateShaderProgram() = 0;
	virtual EShaderError				AttachAndCompileShaderFromFile(const char * filename, EShaderType type) = 0;
	virtual void						BuildShaderProgram() = 0;
	virtual void						DeleteShaderProgram() = 0;
	virtual void						ActivateShaderProgram() = 0;
	virtual void						DeactivateShaderProgram() = 0;

	//todo: delete!
	virtual void						SetUniform1UInt(unsigned int value, const char* param) {};
	virtual void						SetAttrib4Float(float f1, float f2, float f3, float f4, const char* param) {};
	virtual void						BindAttributeLocation(int index, const char* param_name) {};
	struct Shader_Define
	{
		Shader_Define(std::string name,std::string value)
		{
			Name = name;
			Value = value;
		}
		std::string Name;
		std::string Value;
	};
	void								ModifyCompileEnviroment(Shader_Define Define);
protected:
	std::vector<Shader_Define> Defines;
};

