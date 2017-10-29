#pragma once
#include "Light.h"
#include <vector>
#include "Rendering\Shaders\Shader_Depth.h"
#include <GLEW\GL\glew.h>
#include <memory>
#include "Core/GameObject.h"

class ShadowRenderer
{
public:
	ShadowRenderer();
	~ShadowRenderer();
	const int MAX_POINT_SHADOWS = 2;
	const int MAX_DIRECTIONAL_SHADOWS = 4;
	void RenderShadowMaps(Camera * c, std::vector<Light*> lights, std::vector<GameObject*>& ShadowObjects);
	void BindShadowMaps();
	void InitShadows(std::vector<Light*> lights);
	bool UseCache = true;
	bool Renderered = false;
private:

	std::vector<Shader_Depth*> ShadowShaders;
};

