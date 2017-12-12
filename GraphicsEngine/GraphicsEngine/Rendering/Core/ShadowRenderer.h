#pragma once
#include "Light.h"
#include <vector>
#include "Rendering\Shaders\Shader_Depth.h"
#include <GLEW\GL\glew.h>
#include <memory>
#include "Core/GameObject.h"
#include "../EngineGlobals.h"
class ShadowRenderer
{
public:
	ShadowRenderer();
	~ShadowRenderer();
	const int MAX_POINT_SHADOWS = 2;
	const int MAX_DIRECTIONAL_SHADOWS = 4;
	void RenderShadowMaps(Camera * c, std::vector<Light*> lights, const std::vector<GameObject*>& ShadowObjects,CommandListDef* list = nullptr, class Shader_Main* mainshader = nullptr);
	void BindShadowMaps(CommandListDef* list = nullptr);
	void ClearShadowLights();
	void InitShadows(std::vector<Light*> lights);
	bool UseCache = true;
	bool Renderered = false;
	CommandListDef* CreateShaderCommandList();
	void ResetCommandList(CommandListDef* list);

private:

	std::vector<Shader_Depth*> ShadowShaders;
};

