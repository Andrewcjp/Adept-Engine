#include "Camera.h"
#include "Core/Transform.h"
#include "Editor/EditorWindow.h"

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
	if (isnan(aspect))
	{
		return;
	}
	AspectRatio = aspect;
	if (UseLeftHanded)
	{
		this->projection = glm::perspectiveLH(glm::radians(fov), aspect, zNear, ZFar);
	}
	else
	{
		this->projection = glm::perspective(glm::radians(fov), aspect, zNear, ZFar);
	}
}

glm::mat4 Camera::GetTransformMatrix()
{
	glm::mat4 posMat = glm::translate(GetPosition());
	glm::mat4 viewmat = glm::inverse(GetView());
	glm::mat4 qrotation = glm::lookAt(glm::vec3(0, 0, 0), glm::normalize(up), (forward));
	return posMat * qrotation;

}

void Camera::SetUpAndForward(glm::vec3 fward, glm::vec3 uup)
{
	this->forward = fward;
	this->up = uup;
}

glm::mat4 Camera::GetView()
{
	if (Override)
	{
		return oVioew;
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
	m_pos += forward * amt*m_movespeed;
}

void Camera::MoveRight(float amt)
{
	if (UseLeftHanded)
	{
		amt = -amt;
	}
	m_pos += glm::cross(up, forward) * amt*m_movespeed;
}

void Camera::MoveUp(float amt)
{
	m_pos.y += amt * m_movespeed;
}
//Depricated
void Camera::Pitch(float angle)
{
	//Log::OutS  << "pitching by :" << angle << Log::OutS;
	glm::vec3 right = glm::normalize(glm::cross(up, forward));

	forward = glm::vec3(glm::normalize(glm::rotate(angle, right) * glm::vec4(forward, 0.0)));
	up = glm::normalize(glm::cross(forward, right));
}

void Camera::RotateY(float angle)
{
	static const glm::vec3 UP(0.0f, 1.0f, 0.0f);
	if (UseLeftHanded)
	{
		angle = -angle;
	}
	glm::mat4 irotation = glm::rotate(angle, UP);

	forward = glm::vec3(glm::normalize(irotation * glm::vec4(forward, 0.0)));
	up = glm::vec3(glm::normalize(irotation * glm::vec4(up, 0.0)));
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

	/*double x = 2.0 * screenX / EditorWindow::GetWidth() - 1;
	double y = -2.0 * screenY / EditorWindow::GetHeight() + 1;*/
	float x = (2.0f * screenX) / BaseWindow::GetWidth() - 1.0f;
	float y = 1.0f - (2.0f * screenY) / BaseWindow::GetHeight();

	glm::mat4x4 viewProjectionInverse = glm::inverse(projection *
		GetView());

	glm::vec3 point3D = glm::vec3(x, y, 0.0f);
	return viewProjectionInverse * glm::vec4(point3D.xyz, 0.0f);
}
#include "Core/Utils/MathUtils.h"
void Camera::SetPos(glm::vec3 value)
{
	CheckNAN(value);

	m_pos = value;
}

//old
//glm::mat4 inverseVP = glm::inverse(GetProjection() * GetView());
//
////#if OGRE_NO_VIEWPORT_ORIENTATIONMODE == 0
////	// We need to convert screen point to our oriented viewport (temp solution)
////	Real tX = screenX; Real a = getOrientationMode() * Math::HALF_PI;
////	screenX = Math::Cos(a) * (tX - 0.5f) + Math::Sin(a) * (screenY - 0.5f) + 0.5f;
////	screenY = Math::Sin(a) * (tX - 0.5f) + Math::Cos(a) * (screenY - 0.5f) + 0.5f;
////	if ((int)getOrientationMode() & 1) screenY = 1.f - screenY;
////#endif
//
//float nx = (2.0f * screenX) - 1.0f;
//float ny = 1.0f - (2.0f * screenY);
//glm::vec4 nearPoint(nx, ny, -1.f, 0);
//// Use midPoint rather than far point to avoid issues with infinite projection
//glm::vec4  midPoint(nx, ny, 0.0f, 0);
//
//// Get ray origin and ray target on near plane in world space
//glm::vec4  rayOrigin, rayTarget;
//
//rayOrigin = inverseVP * nearPoint;
//rayTarget = inverseVP * midPoint;
//outrayDirection = rayTarget;
////glm::normalize(outrayDirection);
//outRayorign = rayOrigin;
///*outRay->setOrigin(rayOrigin);
//outRay->setDirection(rayDirection); */