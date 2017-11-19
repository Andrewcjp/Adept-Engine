#include <stdio.h>
#include <stdlib.h>
#include <GLEW\GL\glew.h>
#include "../Core/Assets/AssetManager.h"
#include "OGLShader.h"
#include "../Core/Utils/WindowsHelper.h"
#include "../Core/Utils/StringUtil.h"

OGLShader::OGLShader()
{
	m_ownerProgram = INVALID_SHADER_PROGRAM_HANDLE;
}

OGLShader::~OGLShader()
{
	DeleteShader();
}

EShaderError OGLShader::CreateShaderFromSourceFile(const char * filename, EShaderType type)
{
	std::string path = filename;// "../asset/shader/glsl/";//fallback.frag
	//path.append(filename);
	switch (type)
	{
	case SHADER_FRAGMENT:
		path.append(".frag");
		break;
	case SHADER_VERTEX:
		path.append(".vert");
		break;
	case SHADER_GEOMETRY:
		path.append(".geo");
		break;
	}
	m_shaderInternal._type = type;

	if (type == SHADER_UNDEFINED)
		return SHADER_ERROR_CREATE;

	GLhandleARB shaderhandle;
	GLenum shadertype;

	switch (type)
	{
	case SHADER_VERTEX: shadertype = GL_VERTEX_SHADER; break;
	case SHADER_FRAGMENT: shadertype = GL_FRAGMENT_SHADER; break;
	case SHADER_GEOMETRY: shadertype = GL_GEOMETRY_SHADER; break;
	case SHADER_UNDEFINED: break;
	}

	//Create a shader handle from the device.
	shaderhandle = glCreateShader(shadertype);

	if (shaderhandle == 0 || shaderhandle == GL_INVALID_ENUM || shaderhandle == GL_INVALID_OPERATION)
	{
		return SHADER_ERROR_CREATE;
	}

	m_shaderInternal._syshandle = shaderhandle;	//store the shader handle.

	//TODO: try using AUTO_PTR here?

	//size_t length = TextFileBufferedRead(path, &source);
	//std::string length = AssetManager::GetShaderAsset(path, &source);
	std::string data = AssetManager::instance->LoadFileWithInclude(path);

	//upload the source code to the shader program
	if (data.length() > 0)
	{
		const char *c_str = data.c_str();
		glShaderSource(shaderhandle, 1, &c_str, NULL);
	}
	else
	{
		//there is no file
		return SHADER_ERROR_NOFILE;
	}
	//delete[] source;

	//TODO: Search documents to find out the way to catch shader compilation errors etc.

	//compile the bugger.
	GLint result = 0;
	glCompileShader(shaderhandle);
	glGetShaderiv(shaderhandle, GL_INFO_LOG_LENGTH, &result);

	if (result > 0)
	{
		char* log = new char[result];
		int charlen = 0;
		glGetShaderInfoLog(shaderhandle, result, &charlen, log);

		fprintf(stdout, "Shader log %s\n", log);
		std::string Log = "Shader: ";
		Log.append(filename);
		Log.append("\n");
		Log.append(log);
		WindowsHelpers::DisplayMessageBox("Shader Complie Error", Log);
		delete[] log;
	}
	else
	{

		//	fprintf(stdout, "Shader %s has been successfully compiled.\n", path.c_str());
	}

	return SHADER_ERROR_NONE;
}

void OGLShader::DeleteShader()
{
	ShaderHandle handle = m_shaderInternal._syshandle;

	if (handle != INVALID_SHADER_HANDLE)
	{
		if (m_ownerProgram != INVALID_SHADER_PROGRAM_HANDLE)
		{
			glDetachObjectARB(m_ownerProgram, m_shaderInternal._syshandle);
		}
		glDeleteObjectARB(handle);
		m_shaderInternal._syshandle = INVALID_SHADER_HANDLE;
		m_shaderInternal._handle = INVALID_SHADER_HANDLE;
		m_shaderInternal._type = SHADER_UNDEFINED;
		m_shaderInternal._name[0] = '\0';
		m_ownerProgram = INVALID_SHADER_PROGRAM_HANDLE;
	}
}

void OGLShader::SetShaderOwnerProgram(ShaderProgramHandle handle)
{
	m_ownerProgram = handle;
}
// END_OF ShaderGL
//
// BEGINNING_OF ShaderProgramGL
