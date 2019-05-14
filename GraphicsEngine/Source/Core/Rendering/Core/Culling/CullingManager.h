#pragma once
#include "CullingFrustum.h"

class Scene;
class CullingManager
{
public:
	CullingManager();
	~CullingManager();
	void DebugRender(Camera * maincam, Scene* target);
	void UpdateMainPassFrustumCulling(Camera * maincam, Scene * target);
	void UpdateCullingForShadowLight(Light * light,Scene* target);
private:
	CullingFrustum Frustum;
	
};

