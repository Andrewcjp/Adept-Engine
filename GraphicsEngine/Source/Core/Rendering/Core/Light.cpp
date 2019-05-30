#include "Light.h"
int Light::LastId = 0;
Light::Light(glm::vec3 positon, float intesity, ELightType::Type type, glm::vec3 LightColor, bool doesshadow)
{
	m_position = positon;
	m_intesity = intesity;
	m_lightColor = LightColor;
	m_type = type;
	DoesShadow = doesshadow;
	Resolution = RHI::GetRenderSettings()->GetShadowSettings().MaxShadowMapSize;
}

Light::~Light()
{}

glm::vec3 Light::GetPosition() const
{
	return m_position;
}

glm::vec3 Light::GetColor() const
{
	return m_lightColor * m_intesity;
}

ELightType::Type Light::GetType() const
{
	return m_type;
}

void Light::SetPostion(glm::vec3 pos)
{
	m_position = pos;
}

void Light::SetIntesity(float value)
{
	m_intesity = value;
}

void Light::SetLightType(ELightType::Type value)
{
	m_type = value;
}

void Light::SetShadow(bool state)
{
	DoesShadow = state;
}

void Light::SetDirection(const glm::vec3 & value)
{
	m_direction = glm::normalize(value);
}

bool Light::GetDoesShadow() const
{
	return DoesShadow;
}

void Light::SetShadowId(int id)
{
	ShadowId = id;
}

int Light::GetShadowId() const
{
	return ShadowId;
}

glm::vec3 Light::GetDirection() const
{
	return m_direction;
}

float Light::GetIntesity()
{
	return m_intesity;
}

void Light::SetShadowResdent(int DeviceIndex, int CopyTarget)
{
	GPUShadowResidentMask[DeviceIndex] = true;
	GPUShadowResidentMask[CopyTarget] = false;
	GPUShadowCopyDeviceTarget[DeviceIndex] = CopyTarget;
}

void Light::Update()
{
	//1.0 / (distanceToLight * distanceToLight);
	float minValue = 0.001f;
	FalloffRange = (minValue / m_intesity);
	FalloffRange = glm::sqrt(1.0f / FalloffRange);
}

float Light::GetRange()
{
	return glm::min(FalloffRange, Distance);
}

ELightMode::Type Light::GetLightMode()
{
	return LightMode;
}

void Light::SetLightMode(ELightMode::Type t)
{
	LightMode = t;
}

inline EShadowCaptureType::Type Light::GetShadowMode() const
{
	return ShadowMode;
}

inline void Light::SetShadowMode(EShadowCaptureType::Type val)
{
	ShadowMode = val;
}
