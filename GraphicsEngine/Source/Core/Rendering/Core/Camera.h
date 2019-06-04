#pragma once
#include "RHI/RHI.h"
class Transform;
class Camera
{
public:
	Camera();
	CORE_API Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar);
	bool UseLeftHanded = false;
	float maxz = 50;
	float zNear = 0.0f;
	float ZFar = 1000.0f;
	float fov = 70.0f;
	CORE_API glm::mat4 GetViewProjection();
	void UpdateProjection(float aspect);
	glm::mat4 GetTransformMatrix();
	CORE_API glm::mat4 GetProjection();
	CORE_API void SetUpAndForward(glm::vec3 fward, glm::vec3 uup);
	CORE_API glm::mat4 GetView();
	CORE_API glm::vec3 GetPosition();

	void GetRayAtScreenPos(float screenX, float screenY, glm::vec3 & outrayDirection, glm::vec3 & outRayorign);
	glm::vec3 ScreenPointToWorld(float screenx, float screeny);
	glm::vec3 TransformDirection(glm::vec3 pDirection, glm::mat4 pMatrix);
	CORE_API glm::vec3 GetRight();
	CORE_API glm::vec3 GetUp();
	CORE_API glm::vec3 GetForward();
	CORE_API void SetPos(glm::vec3 pos);
	void Sync(Transform* t);
	float AspectRatio = 1.0f;
	void SetViewTransFrom(glm::mat4 ViewTransfrom);
	void SetProjection(glm::mat4 ViewTransfrom);
	SceneLayerMask RenderMask;
protected:

private:
	bool Override = false;
	bool proj = false;

	glm::mat4 projection;
	glm::vec3 m_pos;
	glm::vec3 forward;
	glm::vec3 up;
	glm::quat qrot;
	glm::vec3 rotation;
	glm::mat4 ViewOverdide;
};

