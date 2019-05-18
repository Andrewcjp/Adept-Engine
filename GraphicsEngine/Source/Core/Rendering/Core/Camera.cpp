#include "Camera.h"
#include "Core/Transform.h"
#include "Editor/EditorWindow.h"
#include "Core/Utils/MathUtils.h"

Camera::Camera()
{
	UseLeftHanded = (RHI::GetType() == RenderSystemD3D11 || RHI::GetType() == RenderSystemD3D12);
	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	ZFar = 1000;
	zNear = 0.1f;
	fov = 90;
	UpdateProjection(1);
}

Camera::Camera(glm::vec3 pos, float fov, float aspect, float zNear, float zFar) :Camera()
{
	m_pos = pos;
	zNear = zNear;
	ZFar = zFar;
	fov = fov;
	UpdateProjection(aspect);
}

glm::mat4 Camera::GetViewProjection()
{
	return projection * GetView();
}

void Camera::UpdateProjection(float aspect)
{
	if (isnan(aspect))
	{
		return;
	}
	if (proj)
	{
		return;
	}
	AspectRatio = aspect;
	if (UseLeftHanded)
	{
		projection = glm::perspectiveLH(glm::radians(fov), aspect, zNear, ZFar);
	}
	else
	{
		projection = glm::perspective(glm::radians(fov), aspect, zNear, ZFar);
	}
}

glm::mat4 Camera::GetTransformMatrix()
{
	glm::mat4 posMat = glm::translate(GetPosition());
	glm::mat4 viewmat = glm::inverse(GetView());
	glm::mat4 qrotation = glm::lookAt(glm::vec3(0, 0, 0), glm::normalize(up), (forward));
	return posMat * qrotation;

}

glm::mat4 Camera::GetProjection()
{
	return projection;
}

void Camera::SetUpAndForward(glm::vec3 fward, glm::vec3 uup)
{
	forward = fward;
	up = uup;
}

glm::mat4 Camera::GetView()
{
	glm::mat4 output;
	if (Override)
	{
		return ViewOverdide;
	}
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
	return m_pos;
}

void Camera::GetRayAtScreenPos(float  screenX, float  screenY, glm::vec3&  outrayDirection, glm::vec3&  outRayorign)
{
	float x = (2.0f * screenX) / BaseWindow::GetWidth() - 1.0f;
	float y = 1.0f - (2.0f * screenY) / BaseWindow::GetHeight();

	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);
	glm::vec4 ray_clip = glm::vec4(ray_nds.xy, -1.0, 1.0);
	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.xy, -1.0, 0.0);

	glm::vec3 ray_wor = (glm::inverse(GetView()) * ray_eye).xyz;
	// don't forget to normalise the vector at some point
	ray_wor = glm::normalize(ray_wor);

	outrayDirection = ray_wor;
	outRayorign = GetPosition();
}

glm::vec3  Camera::ScreenPointToWorld(float screenX, float screenY)
{
	float x = (2.0f * screenX) / BaseWindow::GetWidth() - 1.0f;
	float y = 1.0f - (2.0f * screenY) / BaseWindow::GetHeight();

	glm::mat4x4 viewProjectionInverse = glm::inverse(projection *
		GetView());

	glm::vec3 point3D = glm::vec3(x, y, 0.0f);
	return viewProjectionInverse * glm::vec4(point3D.xyz, 0.0f);
}

glm::vec3 Camera::TransformDirection(glm::vec3 pDirection, glm::mat4 pMatrix)
{
	return glm::normalize(glm::vec3(pMatrix * glm::vec4(pDirection, 0.0f)));
}

glm::vec3 Camera::GetRight()
{
	return glm::normalize(glm::cross(up, forward));
}

glm::vec3 Camera::GetUp()
{
	return up;
}

glm::vec3 Camera::GetForward()
{
	return forward;
}

void Camera::SetPos(glm::vec3 pos)
{
	m_pos = pos;
}

void Camera::Sync(Transform * t)
{
	m_pos = t->GetPos();
	SetUpAndForward(t->GetForward(), t->GetUp());
}

void Camera::SetViewTransFrom(glm::mat4 ViewTransfrom)
{
	Override = true;
	ViewOverdide = ViewTransfrom;
}
void Camera::SetProjection(glm::mat4 ViewTransfrom)
{
	projection = ViewTransfrom;
	proj = true;
}
