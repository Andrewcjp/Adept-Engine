#include "Shader_VRRResolve.h"

IMPLEMENT_GLOBAL_SHADER(Shader_VRRResolve);
Shader_VRRResolve::Shader_VRRResolve(DeviceContext * device) :Shader(device)
{
	m_Shader->AttachAndCompileShaderFromFile("VRX\\VRRResolve", EShaderType::SHADER_COMPUTE);

}

Shader_VRRResolve::~Shader_VRRResolve()
{}

