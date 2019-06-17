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
		PreSampleShadows,
		ScreenSpecular,
		Limit
	};
}
class Shader_Deferred :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Deferred);
	Shader_Deferred(class DeviceContext* dev);
	~Shader_Deferred();

	std::vector<ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	void RenderScreenQuad(RHICommandList * list);

private:
	RHIBuffer * VertexBuffer = nullptr;
};

