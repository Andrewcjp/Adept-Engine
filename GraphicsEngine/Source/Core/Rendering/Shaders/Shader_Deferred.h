#pragma once
#include "RHI/Shader.h"
namespace DeferredLightingShaderRSBinds
{
	enum type
	{
		LightDataCBV = 0,
		MVCBV,
		PosTex,
		NormalTex,
		AlbedoTex,
		DirShadow,
		PointShadow,
		DiffuseIr,
		SpecBlurMap,
		EnvBRDF,
		Limit
	};
}
class Shader_Deferred :
	public Shader
{
public:
	Shader_Deferred();
	~Shader_Deferred();

	std::vector<Shader::ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	void RenderScreenQuad(RHICommandList * list);

private:
	RHIBuffer * VertexBuffer = nullptr;
};

