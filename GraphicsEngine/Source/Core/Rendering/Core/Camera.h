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
	glm::mat4 GetViewProjection();
	void UpdateProjection(float aspect);
	glm::mat4 GetTransformMatrix();
	glm::mat4 GetProjection()
	{
		return projection;
	}
	CORE_API void SetUpAndForward(glm::vec3 fward, glm::vec3 uup);
	glm::mat4 GetView();
	CORE_API glm::vec3 GetPosition();
	void MoveForward(float amt);
	void MoveRight(float amt);
	void MoveUp(float amt);
	CORE_API void Pitch(float angle);
	CORE_API void RotateY(float angle);
	void ePitch(float angle);
	void qPitch(float angle);
	void qRotateY(float angle);
	void eRotateY(float angle);
	void SetMouseRotation(float x, float y);
	void GetRayAtScreenPos(float screenX, float screenY, glm::vec3 & outrayDirection, glm::vec3 & outRayorign);
	glm::vec3 ScreenPointToWorld(float screenx, float screeny);
	glm::vec3 TransformDirection(glm::vec3 pDirection, glm::mat4 pMatrix)
	{
		return glm::normalize(glm::vec3(pMatrix * glm::vec4(pDirection, 0.0f)));
	}
	void LinkToTransform(Transform* t)
	{
		linkedtransform = t;
	}
	CORE_API void SetPos(glm::vec3 value);
	glm::vec3 GetRight()
	{
		return glm::normalize(glm::cross(up, forward));
	}
	glm::vec3 GetUp()
	{
		return up;
	}
	glm::vec3 GetForward()
	{
		return forward;
	}
	bool RenderMainPass = false;
	bool isfree = false;
	void OverrideVP(glm::mat4 v, glm::mat4 p)
	{
		Override = true;
		projection = p;
		oVioew = v;
	}
	void Sync(Transform* t);
	float AspectRatio = 1.0f;
protected:

private:
	bool Override = false;
	glm::mat4 projection;
	glm::vec3 m_pos;
	glm::vec3 forward;
	glm::vec3 up;
	glm::quat qrot;
	glm::vec3 rotation;
	float m_movespeed = 0.1f;
	Transform* linkedtransform = nullptr;
	//debug
	glm::mat4 oVioew;
};

