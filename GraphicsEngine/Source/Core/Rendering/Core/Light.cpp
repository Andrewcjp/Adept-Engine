#include "Light.h"
#include "RHI\DeviceContext.h"
int Light::LastId = 0;
Light::Light(glm::vec3 positon, float intesity, ELightType::Type type, glm::vec3 LightColor, bool doesshadow)
{
	m_position = positon;
	m_intesity = intesity;
	m_lightColor = LightColor;
	m_type = type;
	DoesShadow = doesshadow;
	Resolution = RHI::GetRenderSettings()->GetShadowSettings().DefaultShadowMapSize;
	//ShadowMode = ELightMobility::Baked;
	InvalidateCachedShadow();
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


void Light::Update()
{
	//this needs to be in sync with shaders or the light culling broadphase will cause issues.
	FalloffRange = (MinLightIntensity / m_intesity);
	FalloffRange = glm::sqrt(1.0f / FalloffRange);
	FalloffRange = Distance;
}

float Light::GetRange()
{
	return glm::min(FalloffRange, Distance);
}

ELightMobility::Type Light::GetLightMobility() const
{
	return ShadowMode;
}

void Light::SetShadowMode(ELightMobility::Type val)
{
	ShadowMode = val;
}

void Light::SetShadowResidentToSingleGPU(int DeviceIndex, int CopyTarget)
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		GPUResidenceMask[i].IsPresentOnGPU = false;
	}
	GPUResidenceMask[DeviceIndex].IsPresentOnGPU = true;
	//Target all GPUs - the present one is auto excluded.
	GPUResidenceMask[DeviceIndex].GPUTargetFlags.SetFlags(0xfffffff);
}

LightGPUAffinty* Light::FindLightResident()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (GPUResidenceMask[i].IsPresentOnGPU)
		{
			return &GPUResidenceMask[i];
		}
	}
	return nullptr;
}


bool Light::ShouldCopyToDevice(int index)
{
	return FindLightResident()->GPUTargetFlags.GetFlagValue(index);
}

bool Light::NeedsShadowUpdate(int deviceIndex)
{
	if (!DoesShadow)
	{
		return false;
	}
	if (GetLightMobility() == ELightMobility::Realtime)
	{
		return true;
	}
	return ShadowNeedsUpdate[deviceIndex];
}

void Light::InvalidateCachedShadow()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		ShadowNeedsUpdate[i] = true;
	}
}

void Light::NotifyShadowUpdate(int deviceIndex)
{
	ShadowNeedsUpdate[deviceIndex] = false;
}

bool Light::IsResident(DeviceContext * dev)
{
	return GPUResidenceMask[dev->GetDeviceIndex()].IsPresentOnGPU;
}

bool Light::HasValidHandle(int deviceindex)
{
	if (GPUResidenceMask[deviceindex].AtlasHandle == nullptr)
	{
		return false;
	}
	//#Shadow: more validity checks

	return true;
}
