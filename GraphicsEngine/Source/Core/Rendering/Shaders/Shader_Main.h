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
		Limit
	};
}

class Shader_Main :public Shader
{
public:
	Shader_Main(bool LoadForward = true);
	~Shader_Main();
	static void GetMainShaderSig(std::vector<ShaderParameter>& out);
	static std::vector<Shader::VertexElementDESC> GetMainVertexFormat();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<ShaderParameter> GetShaderParameters() override;
};

