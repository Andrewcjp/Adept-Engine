#pragma once
#include "RHI/Shader.h"
#define CUBE_SIDES 6
#include "Rendering/Core/FrameBuffer.h"

namespace Shader_Depth_RSSlots
{
	enum Type
	{
		ModelBuffer = 0,
		GeometryProjections = 1,
		VPBuffer = 2,
		VI_Offset,
	};
}
class Shader_Depth :public Shader
{
public:
	DECLARE_GLOBAL_SHADER_ARGS(Shader_Depth, bool);
	DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_Depth_ON, Shader_Depth, bool, true);
	DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_Depth_OFF, Shader_Depth, bool, false);
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
	std::vector<ShaderParameter> GetShaderParameters() override;
	void UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index);
	void SetProjections(RHICommandList* list, int index);
	bool LoadGeomShader = true;
private:
	RHIBuffer * ConstantBuffer = nullptr;
	float znear = 1;
	float zfar = 50;
	RHIBuffer* GeometryProjections = nullptr;
};

