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
	bool RightParallelepipedInFrustum(const glm::vec4& Min,const glm::vec4& Max);
	void DebugRender();
	bool SphereInFrustum(const glm::vec3 & pos, float radius);
	void normalizePlane(glm::vec4 & frustum_plane);
	void CalculateFrustum(const glm::mat4 & view_matrix, const  glm::mat4 & proj_matrix);
private:
	float nearD, farD, ratio, angle, tang;
	float NearWidth, NearHeight, FarWidth, FarHeight;
	glm::vec4 frustum_planes[6];
	DebugDrawers::FrustumData Frustum;
};

