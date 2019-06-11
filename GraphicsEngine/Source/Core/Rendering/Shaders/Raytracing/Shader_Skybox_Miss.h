#pragma once
#include "Rendering/RayTracing/Shader_RTBase.h"

class Shader_Skybox_Miss :public Shader_RTBase
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Skybox_Miss);
	Shader_Skybox_Miss(DeviceContext* Con);
	~Shader_Skybox_Miss();
	void SetSkybox(BaseTextureRef T);
	virtual std::vector<ShaderParameter> GetShaderParameters() override;

};

