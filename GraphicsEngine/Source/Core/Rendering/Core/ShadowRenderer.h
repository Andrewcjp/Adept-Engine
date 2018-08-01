#pragma once
#include <vector>
#include "EngineGlobals.h"
#include "RHI/RHI_inc_fwd.h"

class ShadowRenderer
{
public:
	ShadowRenderer();
	~ShadowRenderer();
	void UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index);
	void RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects,class Shader_Main* mainshader = nullptr);
	void RunPointShadowPass(RHICommandList * List, const std::vector<GameObject*>& ShadowObjects, Shader_Main * mainshader);
	void PreSampleShadows(const std::vector<GameObject*>& ShadowObjects, Shader_Main * mainshader);
	void RenderPointShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject*>& ShadowObjects);
	void RenderDirectionalShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects);
	void BindShadowMapsToTextures(RHICommandList* list);
	void ClearShadowLights();
	void InitShadows(std::vector<Light*> lights);
	bool UseCache = false;
	bool Renderered = false;
	void Unbind(RHICommandList* list);
private:
	std::vector<Light*> ShadowingDirectionalLights;
	std::vector<Light*> ShadowingPointLights;

	Shader_Depth* PointLightShader = nullptr;
	Shader_Depth* DirectionalLightShader = nullptr;

	FrameBuffer* DirectionalLightBuffer = nullptr;
	std::vector<FrameBuffer*> DirectionalLightBuffers;
//	std::vector <FrameBuffer*> PointLightBuffers;
	RHICommandList* PointShadowList = nullptr;
	RHICommandList* PointShadowListALT = nullptr;
	RHICommandList*  DirectionalShadowList = nullptr;
	RHITextureArray* ShadowCubeArray = nullptr;
	RHITextureArray* ShadowDirectionalArray = nullptr;
	//Should be on both devices as constant data
	RHIBuffer* GeometryProjections = nullptr;
	
	struct ShadowLightInteraction
	{
		ShadowLightInteraction(class DeviceContext * Context, bool IsPoint, int MapSize);
		~ShadowLightInteraction();
		void PreSampleShadows(RHICommandList* List);
		FrameBuffer* ShadowMap = nullptr;
		Shader_Depth* Shader = nullptr;
	
		//PreSampled Buffer used to reduce Daa transfer
		FrameBuffer* PreSampledBuffer = nullptr;

		bool NeedsSample = false;
		bool IsPointLight = false;
		int DeviceIndex = 0;
	};
	std::vector<ShadowLightInteraction*> LightInteractions;

	
	class Shader_ShadowSample* ShadowPreSampleShader = nullptr;
	RHICommandList* ShadowPreSamplingList = nullptr;
};

