#include "OGLShaderProgram.h"
#include <GLEW\GL\glew.h>
#include "../Core/Utils/StringUtil.h"
OGLShaderProgram::~OGLShaderProgram()
{
	DeleteShaderProgram();
}

void OGLShaderProgram::CreateShaderProgram()
{
	if (m_systemHandle != INVALID_SHADER_PROGRAM_HANDLE) //A shader program has already been created.
		return;

	//TODO: Validate the program handle.
	m_systemHandle = glCreateProgram();
}

void OGLShaderProgram::AttachShader(ShaderBase* shader)
{
	if (m_systemHandle != INVALID_SHADER_PROGRAM_HANDLE && shader->GetShaderSysHandle() != INVALID_SHADER_HANDLE)
	{
		m_shaders[m_shaderCount] = shader;
		m_shaderCount += 1;
		shader->SetShaderOwnerProgram(m_systemHandle);

		glAttachShader(m_systemHandle, shader->GetShaderSysHandle());
	}
}

void OGLShaderProgram::BuildShaderProgram()
{
	GLint result = 0;

	glLinkProgram(m_systemHandle);

	glGetProgramiv(m_systemHandle, GL_INFO_LOG_LENGTH, &result);

	if (result > 0)
	{
		char* log = new char[result];
		int outlength;
		glGetProgramInfoLog(m_systemHandle, result, &outlength, log);

		fprintf(stdout, "Link log: %s\n", log);
		MessageBox(
			NULL,
			StringUtils::ConvertStringToWide(log).c_str(),
			L"Shader Program Link Error",
			MB_ICONERROR
		);
		//__debugbreak();
		delete[] log;
	}
	else
	{
		std::cout << "Shader Program Compiled Sucessfully" << std::endl;
	}

}

void OGLShaderProgram::DeleteShaderProgram()
{
	int numAttached = m_shaderCount;

	if (m_systemHandle != INVALID_SHADER_PROGRAM_HANDLE)
	{
		for (int i = 0; i < numAttached; i++)
		{
			delete m_shaders[i];
			m_shaders[i] = NULL;
		}

		glDeleteObjectARB(m_systemHandle);
		m_systemHandle = INVALID_SHADER_PROGRAM_HANDLE;
	}
}

EShaderError OGLShaderProgram::AttachAndCompileShaderFromFile(const char* filename, EShaderType type)
{
	if (m_shaderCount == MAX_NUM_SHADER_PER_PROGRAM)
		return SHADER_ERROR_MAXCOUNT;

	ShaderBase* shader = new OGLShader();

	if (shader)
	{
		EShaderError state = shader->CreateShaderFromSourceFile(filename, type);
		if (state == SHADER_ERROR_NONE)
		{
			AttachShader(shader);
		}
		else
		{
			if (state == SHADER_ERROR_NOFILE)
			{
				std::cout << "ERROR: Failed to Find File " << filename << std::endl;
			}
			else
			{
				std::cout << "ERROR: Failed to Create Shader" << std::endl;
			}

			delete shader;
		}
	}
	return SHADER_ERROR_NONE;
}
void OGLShaderProgram::ActivateShaderProgram()
{
	if (m_systemHandle != INVALID_SHADER_PROGRAM_HANDLE)
		glUseProgram(m_systemHandle);
}

void OGLShaderProgram::DeactivateShaderProgram()
{
	if (m_systemHandle != INVALID_SHADER_PROGRAM_HANDLE)
		glUseProgram(0);
}

void OGLShaderProgram::SetUniform1UInt(unsigned int value, const char* param)
{
	int param_loc = glGetUniformLocationARB(m_systemHandle, param);

	glUniform1iARB(param_loc, value);
}

void OGLShaderProgram::SetAttrib4Float(float f1, float f2, float f3, float f4, const char* param)
{
	int param_loc = glGetAttribLocationARB(m_systemHandle, param);

	glVertexAttrib4fARB(param_loc, f1, f2, f3, f4);
}

void OGLShaderProgram::BindAttributeLocation(int index, const char* param_name)
{
	glBindAttribLocation(m_systemHandle, index, param_name);
}


