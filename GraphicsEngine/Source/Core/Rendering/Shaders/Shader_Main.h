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
		DirShadow = 3,
		PointShadow = 4,
		DiffuseIr = 5,
		SpecBlurMap = 6,
		EnvBRDF = 7,
		Limit
	};
}

class Shader_Main :public Shader
{
public:
	Shader_Main(bool LoadForward = true);
	~Shader_Main();
	static void GetMainShaderSig(std::vector<Shader::ShaderParameter>& out);
	static std::vector<Shader::VertexElementDESC> GetMainVertexFormat();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
};

