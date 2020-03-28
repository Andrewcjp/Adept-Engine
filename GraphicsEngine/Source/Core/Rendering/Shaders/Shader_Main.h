#pragma once
#include "RHI/Shader.h"
#include "RHI/RHICommandList.h"
namespace MainShaderRSBinds
{
	enum type
	{
		GODataCBV = 0,
		LightDataCBV = 1,
		MVCBV = 2,
		MaterialData,
		DirShadow ,
		PointShadow ,
		DiffuseIr,
		SpecBlurMap,
		EnvBRDF,
		PreSampledShadows,
		ResolutionCBV,
		LightBuffer,
		LightDataBuffer,
		Limit
	};
}

class Shader_Main :public Shader
{
public:
	//#todo: remove this
	TEMP_API Shader_Main(bool LoadForward = true);
	~Shader_Main();
	static void GetMainShaderSig(std::vector<ShaderParameter>& out);
	static std::vector<VertexElementDESC> GetMainVertexFormat();
	std::vector<VertexElementDESC> GetVertexFormat() override;
	std::vector<ShaderParameter> GetShaderParameters() override;
	
};

