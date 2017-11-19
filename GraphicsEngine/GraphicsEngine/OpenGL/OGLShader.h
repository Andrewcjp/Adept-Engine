#ifndef __SHADERGL_H__
#define __SHADERGL_H__

#include "../RHI/ShaderBase.h"
#include "../Rendering/Core/Camera.h"
#include "Core/Transform.h"
#include "RHI/ShaderProgramBase.h"
class OGLShader : public ShaderBase
{
	private:
		ShaderProgramHandle			m_ownerProgram;
	public:
									OGLShader();
		virtual						~OGLShader();



		virtual EShaderError CreateShaderFromSourceFile(const char * filename, EShaderType type);
		virtual void				DeleteShader();
		virtual void				SetShaderOwnerProgram(ShaderProgramHandle handle);
};


#endif
