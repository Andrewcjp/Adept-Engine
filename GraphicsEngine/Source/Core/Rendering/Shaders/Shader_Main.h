#pragma once
#include "RHI/Shader.h"
#include <string>
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
	void SetNormalVis();
	void SetShadowVis();
	void SetFullBright();
	void RefreshLights() { Once = false; };
	bool ISWATER = false;
	float currentnumber = 0;

	static void GetMainShaderSig(std::vector<Shader::ShaderParameter>& out);
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	

private:
	
	bool shadowvisstate = false;
	bool vistate = false;
	bool enabledFullBright = false;
	_MVPStruct UBuffer;
	/*LightUniformBuffer LightBuffer;*/
	const int CurrentLightcount = 10;
	//LightUniformBuffer* LightBuffers = nullptr;
	bool Once = false;
	//todo move to shader
	
	//int MaxConstant = 125;
	//std::vector<SceneConstantBuffer> SceneBuffer;
	////information for all the lights in the scene currently

	RHIBuffer* CLightBuffer;
	RHIBuffer* CMVBuffer = nullptr;
	RHIBuffer* GameObjectTransformBuffer[MAX_DEVICE_COUNT] = { nullptr };
	//the View and projection Matix in one place as each gameobject will not have diffrent ones.
	//struct MVBuffer MV_Buffer;
	//LightBufferW LightsBuffer;


};

