#pragma once

#include "include\glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
#include "include\glm\gtc\quaternion.hpp"
#include "include\glm\gtx\quaternion.hpp"
#include <iostream>
#include "../RHI/RHI.h"
class Transform;
class Camera
{
public:
	Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar, bool LH = false);
	bool UseLeftHanded = false;
	float maxz = 50;
	float zNear = 0.0f;
	float ZFar = 1000.0f;
	float fov = 70.0f;
	glm::mat4 GetViewProjection();
	void UpdateProjection(float aspect);
	glm::mat4 GetProjection() {
		return projection;
	}
	void SetUpAndForward(glm::vec3 fward, glm::vec3 uup);
	glm::mat4 GetView();
	glm::vec3 GetPosition();
	void MoveForward(float amt);
	void MoveRight(float amt);
	void MoveUp(float amt);
	void Pitch(float angle);
	void RotateY(float angle);
	void ePitch(float angle);
	void qPitch(float angle);
	void qRotateY(float angle);
	void eRotateY(float angle);
	void SetMouseRotation(float x, float y);
	glm::vec3 TransformDirection(glm::vec3 pDirection, glm::mat4 pMatrix) {
		return glm::normalize(glm::vec3(pMatrix * glm::vec4(pDirection, 0.0f)));
	}
	void LinkToTransform(Transform* t) {
		linkedtransform = t;
	}
	void SetPos(glm::vec3 value) {
		m_pos = value;
	}
	glm::vec3 GetRight() {
		return glm::normalize(glm::cross(up, forward));
	}
	glm::vec3 GetUp() {
		return up;
	}
	glm::vec3 GetForward() {
		return forward;
	}
	bool RenderMainPass = false;
	bool isfree = false;
protected:

private:
	glm::mat4 projection;
	glm::vec3 m_pos;
	glm::vec3 forward;
	glm::vec3 up;
	glm::quat qrot;
	glm::vec3 rotation;
	float m_movespeed = 0.1f;
	Transform* linkedtransform;
};

