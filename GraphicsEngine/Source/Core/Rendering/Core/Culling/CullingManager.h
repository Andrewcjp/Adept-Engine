#pragma once
#include "CullingFrustum.h"

class Scene;
class Light;
class CullingManager
{
public:
	CullingManager();
	~CullingManager();
	void DebugRender(Camera * maincam, Scene* target);
	void UpdateMainPassFrustumCulling(Camera * maincam, Scene * target);

	void ProcessObject(GameObject* CurrentObj, Camera * maincam, int& cullcount);

	void UpdateCullingForShadowLight(Light * light, Scene* target);
	CullingFrustum* GetFustrum();
private:
	CullingFrustum Frustum;
	
};

