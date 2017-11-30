#include "Camera.h"
#include "Core/Transform.h"

Camera::Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar)
{
	UseLeftHanded = (RHI::GetType() == RenderSystemD3D11 || RHI::GetType() == RenderSystemD3D12);
	this->m_pos = pos;
	this->forward = glm::vec3(0.0f, 0.0f, 1.0f);
	this->up = glm::vec3(0.0f, 1.0f, 0.0f);
	this->zNear = zNear;
	ZFar = zFar;
	this->fov = fov;
	if (UseLeftHanded)
	{
		this->projection = glm::perspectiveLH(glm::radians(fov), aspect, zNear, zFar);
	}
	else
	{
		this->projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
	}
}

glm::mat4 Camera::GetViewProjection()
{
	return projection * GetView();
}

void Camera::UpdateProjection(float aspect)
{
	if (UseLeftHanded)
	{
		this->projection = glm::perspectiveLH(glm::radians(fov), aspect, zNear, ZFar);
	}
	else
	{
		this->projection = glm::perspective(glm::radians(fov), aspect, zNear, ZFar);
	}
}

void Camera::SetUpAndForward(glm::vec3 fward, glm::vec3 uup)
{
	this->forward = fward;
	this->up = uup;
}

glm::mat4 Camera::GetView()
{
	glm::mat4 rotX = glm::rotate(rotation.x, glm::vec3(1.0, 0.0, 0.0));
	glm::mat4 rotY = glm::rotate(rotation.y, glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 rotZ = glm::rotate(rotation.z, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 rotMat = rotX * rotY * rotZ;
	if (linkedtransform != nullptr && isfree == false)
	{
		//	rotMat *= glm::toMat4(linkedtransform->GetQuatRot());//apply the transform rotation
	}	
	glm::mat4 output;
	if (UseLeftHanded)
	{
		output = glm::lookAtLH(GetPosition(), GetPosition() + forward, up);
	}
	else
	{
		output = glm::lookAt(GetPosition(), GetPosition() + forward, up);
	}
	return output;
}
glm::vec3 Camera::GetPosition()
{
	if (linkedtransform != nullptr && isfree == false)
	{
		return m_pos + linkedtransform->GetPos();
	}
	return m_pos;
}
void Camera::MoveForward(float amt)
{
	m_pos += forward *amt*m_movespeed;
}

void Camera::MoveRight(float amt)
{
	if (UseLeftHanded)
	{
		amt = -amt;
	}
	//printf("%f %f %f", m_pos.x, m_pos.y, m_pos.z);
	m_pos += glm::cross(up, forward) * amt*m_movespeed;
}

void Camera::MoveUp(float amt)
{
	m_pos.y += amt * m_movespeed;
}
//Depricated
void Camera::Pitch(float angle)
{
	//std::cout << "pitching by :" << angle << std::endl;
	glm::vec3 right = glm::normalize(glm::cross(up, forward));

	forward = glm::vec3(glm::normalize(glm::rotate(angle, right) * glm::vec4(forward, 0.0)));
	up = glm::normalize(glm::cross(forward, right));
}
void Camera::qPitch(float angle)
{
	//this->_qrot *= glm::angleAxis((amt), glm::normalize(axis));
	//qrot *= glm::angleAxis((angle), glm::normalize(glm::vec3(0, 0, 1)));
	glm::vec3 euler = glm::eulerAngles(glm::angleAxis((angle), glm::normalize(glm::vec3(0, 0, 1))));
	euler.x = 0;
	qrot *= glm::quat(euler);
}

void Camera::qRotateY(float angle)
{
	//qrot *= glm::angleAxis((angle), glm::normalize(glm::vec3(0, 1, 0)));
	glm::vec3 euler = glm::eulerAngles(glm::angleAxis((angle), glm::normalize(glm::vec3(0, 1, 0))));
	euler.x = 0;
	qrot *= glm::quat(euler);
}
//Depricated
void Camera::RotateY(float angle)
{
	//std::cout << "RotateY by :" << angle << std::endl;
	static const glm::vec3 UP(0.0f, 1.0f, 0.0f);
	if (UseLeftHanded)
	{
		angle = -angle;
	}
	glm::mat4 rotation = glm::rotate(angle, UP);

	forward = glm::vec3(glm::normalize(rotation * glm::vec4(forward, 0.0)));
	up = glm::vec3(glm::normalize(rotation * glm::vec4(up, 0.0)));
}
void Camera::ePitch(float angle)
{
	rotation.z += angle;
	if (rotation.z > maxz)
	{
		rotation.z = maxz;
	}
	if (rotation.z < -maxz)
	{
		rotation.z = -maxz;
	}
}

void Camera::eRotateY(float angle)
{
	rotation.y += angle;
}


void Camera::SetMouseRotation(float x, float y)
{
	forward = glm::vec3(
		cos(x) * sin(y),
		sin(x),
		cos(x) * cos(y)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(y - 3.14f / 2.0f),
		0,
		cos(y - 3.14f / 2.0f)
	);

	// Up vector
	up = glm::cross(right, forward);
}
