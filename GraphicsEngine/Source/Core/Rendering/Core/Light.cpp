#include "Light.h"
int Light::LastId = 0;
Light::Light(glm::vec3 positon, float intesity, LightType type, glm::vec3 LightColor, bool doesshadow)
{
	m_position = positon;
	m_intesity = intesity;
	m_lightColor = LightColor;
	m_type = type;
	DoesShadow = doesshadow;

}

Light::~Light()
{}

void Light::SetShadowResdent(int DeviceIndex, int CopyTarget)
{
	GPUShadowResidentMask[DeviceIndex] = true;
	GPUShadowResidentMask[CopyTarget] = false;
	GPUShadowCopyDeviceTarget[DeviceIndex] = CopyTarget;
}
