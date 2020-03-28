#include "Shader_ParticleDraw.h"
#include "RHI/RHI.h"

IMPLEMENT_GLOBAL_SHADER(Shader_ParticleDraw);
Shader_ParticleDraw::Shader_ParticleDraw(DeviceContext* device) :Shader(device)
{
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleDraw", EShaderType::SHADER_VERTEX, "VSMain");
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleDraw", EShaderType::SHADER_FRAGMENT, "FSMain");
}

Shader_ParticleDraw::~Shader_ParticleDraw()
{}

std::vector<VertexElementDESC> Shader_ParticleDraw::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}


IMP_CACHESHADERPARAM(Shader_ParticleDraw, Texture);
void Shader_ParticleDraw::CacheParms()
{
	CACHEPARM(Texture, Tex);
}
