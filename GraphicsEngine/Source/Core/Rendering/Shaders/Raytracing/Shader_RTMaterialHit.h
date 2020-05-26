#pragma once
#include "Rendering/RayTracing/Shader_RTBase.h"
//this is the template class for material shaders to use 
class Shader_RTMaterialHit : public Shader_RTBase
{
public:
	Shader_RTMaterialHit(DeviceContext* C);
	~Shader_RTMaterialHit();

	virtual std::vector<ShaderParameter> GetShaderParameters() override;

};

