#include "Shader.h"
#include "RHI.h"

Shader::Shader()
{
}

Shader::~Shader()
{

	delete m_Shader;
}

void Shader::UpdateUniforms(Transform * t, Camera * c, std::vector<Light*> lights)
{
	switch (RHI::GetType())
	{
	case RenderSystemOGL:
		UpdateOGLUniforms(t, c, lights);
		break;
	case RenderSystemD3D11:
		UpdateD3D11Uniforms(t, c, lights);
		break;
	}

}

void Shader::UpdateD3D12Uniforms(Transform * , Camera * , std::vector<Light*> lights)
{
}

