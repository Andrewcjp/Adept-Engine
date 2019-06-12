#pragma once
#include "Rendering/RayTracing/Shader_RTBase.h"
//this is the template class for material shaders to use 
class Shader_RTMateralHit : public Shader_RTBase
{
public:
	Shader_RTMateralHit(DeviceContext* C);
	~Shader_RTMateralHit();

	virtual std::vector<ShaderParameter> GetShaderParameters() override;

};

