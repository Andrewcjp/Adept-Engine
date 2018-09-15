#pragma once
#include "RHI/Shader.h"
#include <string>
#define CUBE_SIDES 6
#include "Rendering/Core/FrameBuffer.h"
#include "glm\fwd.hpp"
namespace Shader_Depth_RSSlots
{
	enum Type
	{
		ModelBuffer = 0,
		GeometryProjections = 1,
		VPBuffer = 2,
	};
}
class Shader_Depth :public Shader
{
public:
	struct LightData
	{
		glm::mat4 View;
		glm::mat4 Proj;
		glm::vec3 Lightpos;
	};
	Shader_Depth(bool LoadGeo);
	Shader_Depth(DeviceContext* device, bool LoadGeo);
	void UpdateBuffer(RHICommandList * list, LightData * data, int index);
	~Shader_Depth();
	void SetShaderActive() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	bool LoadGeomShader = true;	
	
private:
	RHIBuffer * ConstantBuffer = nullptr;
	float znear = 1;
	float zfar = 50; 
};

