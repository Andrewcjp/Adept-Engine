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
		LightDataBuffer,
		VX,
		RateImage,
		TempVRRBuffer,
		Limit
	};
}
class Shader_Deferred :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER_ARGS(Shader_Deferred, int);
	Shader_Deferred(class DeviceContext* dev,int VRSMODE);
	~Shader_Deferred();

	std::vector<ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();

};

