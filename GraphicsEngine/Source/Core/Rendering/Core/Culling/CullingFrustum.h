#pragma once
#include "Core\Utils\DebugDrawers.h"

class CullingAABB;
class CullingFrustum
{
public:
	CullingFrustum();
	~CullingFrustum();
	void SetupFromCamera(Camera* cam);
	bool TestObject(CullingAABB* AABB);
	bool RightParallelepipedInFrustum(glm::vec4 Min, glm::vec4 Max);
	void DebugRender();
	void normalizePlane(glm::vec4 & frustum_plane);
	void CalculateFrustum(glm::mat4 & view_matrix, glm::mat4 & proj_matrix);
private:
	float nearD, farD, ratio, angle, tang;
	float NearWidth, NearHeight, FarWidth, FarHeight;
	glm::vec4 frustum_planes[6];
	DebugDrawers::FrustumData Frustum;
};

