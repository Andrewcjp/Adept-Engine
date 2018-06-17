#pragma once
#include <vector>
#include "../EngineGlobals.h"
#include "RHI/RHI_fwd.h"
class ShadowRenderer
{
public:
	ShadowRenderer();
	~ShadowRenderer();
	void UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index);
	void RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects,class Shader_Main* mainshader = nullptr);
	void RenderPointShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject*>& ShadowObjects);
	void RenderDirectionalShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects);
	void BindShadowMapsToTextures(RHICommandList* list);
	void ClearShadowLights();
	void InitShadows(std::vector<Light*> lights);
	bool UseCache = false;
	bool Renderered = false;
private:
	std::vector<Light*> ShadowingDirectionalLights;
	std::vector<Light*> ShadowingPointLights;

	Shader_Depth* PointLightShader = nullptr;
	Shader_Depth* DirectionalLightShader = nullptr;
	RHIBuffer* GeometryProjections = nullptr;
	FrameBuffer* DirectionalLightBuffer = nullptr;
	std::vector<FrameBuffer*> DirectionalLightBuffers;
	std::vector <FrameBuffer*> PointLightBuffers;// = nullptr;
	RHICommandList* PointShadowList = nullptr;
	RHICommandList*  DirectionalShadowList = nullptr;
	RHITextureArray* ShadowCubeArray = nullptr;
	RHITextureArray* ShadowDirectionalArray = nullptr;

};

