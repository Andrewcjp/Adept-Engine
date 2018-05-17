#pragma once
#include "Light.h"
#include <vector>
#include "Rendering\Shaders\Shader_Depth.h"

#include <memory>
#include "Core/GameObject.h"
#include "../EngineGlobals.h"
class ShadowRenderer
{
public:
	ShadowRenderer();
	~ShadowRenderer();
	void UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj);
	const int MAX_POINT_SHADOWS = 2;
	const int MAX_DIRECTIONAL_SHADOWS = 2;
	void RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects, RHICommandList* list = nullptr, class Shader_Main* mainshader = nullptr);
	void RenderPointShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject*>& ShadowObjects);
	void RenderDirectionalShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects);
	void BindShadowMapsToTextures(RHICommandList* list);
	void ClearShadowLights();
	void InitShadows(std::vector<Light*> lights, RHICommandList* list);
	bool UseCache = true;
	bool Renderered = false;
private:
	std::vector<Light*> ShadowingDirectionalLights;
	std::vector<Light*> ShadowingPointLights;

	Shader_Depth* PointLightShader = nullptr;
	Shader_Depth* DirectionalLightShader = nullptr;
	RHIBuffer* GeometryProjections = nullptr;

	//todo: to arrays!
	FrameBuffer* PointLightBuffer = nullptr;
	FrameBuffer* DirectionalLightBuffer = nullptr;
	std::vector<FrameBuffer*> DirectionalLightBuffers;
	RHICommandList* PointShadowList = nullptr;

	bool UseDir2 = false;
};

