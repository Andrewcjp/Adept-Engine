#pragma once
#include "Core/Transform.h"

struct ShadowAtlasHandle;
namespace ELightType
{
	enum Type
	{
		Directional, Point, Spot, Area, Limit
	};
}
namespace ELightMobility
{
	enum Type
	{
		Realtime,//always render everything (uses static caching) 
		Stationary,//does not use lightmapper but captures static objects updates when moved
		Baked,//Light is computed in light mapping
		Limit
	};
};

struct LightGPUAffinty
{
	bool IsPresentOnGPU = true;
	BitFlagsBase GPUTargetFlags;
	//can be different handles on different GPUs
	ShadowAtlasHandle* AtlasHandle = nullptr;

};

class Light
{
public:
	static int LastId;

	//all shadow light make use of dual shadow maps with static caching and composting

	Light(glm::vec3 positon, float intesity, ELightType::Type type = ELightType::Point, glm::vec3 Lightcolor = glm::vec3(1, 1, 1), bool shadow = true);
	~Light();
	glm::vec3 GetPosition() const;
	glm::vec3 GetColor() const;
	ELightType::Type GetType() const;
	void SetPostion(glm::vec3 pos);
	void SetIntesity(float value);
	void SetLightType(ELightType::Type value);
	void SetShadow(bool state);
	void SetDirection(const glm::vec3 &value);
	bool GetDoesShadow() const;
	void SetShadowId(int id);
	int GetShadowId() const;
	glm::vec3 GetDirection()const;
	float GetIntesity();

	void Update();
	float GetRange();

	ELightMobility::Type GetLightMobility() const;
	void SetShadowMode(ELightMobility::Type val);
	float Distance = 512;
	glm::mat4 DirView;
	glm::mat4 Projection;
	glm::vec3 m_lightColor = glm::vec3(1, 1, 1);
	int Resolution = 512;
	//semi realtime lights can use a lower resolution render target for dynamic object and compost into a bigger one
	int BakedResolution = 2048;
	int DirectionalShadowid = -1;
	const float MinLightIntensity = 0.01f;

	//Multi GPU Data
	void SetShadowResidentToSingleGPU(int DeviceIndex, int CopyTarget);
	LightGPUAffinty * FindLightResident();
	bool ShouldCopyToDevice(int index);
	LightGPUAffinty GPUResidenceMask[MAX_GPU_DEVICE_COUNT] = {};
	bool NeedsShadowUpdate(int deviceIndex);
	void InvalidateCachedShadow();
	void NotifyShadowUpdate(int deviceIndex);
	bool IsResident(DeviceContext* dev);
	bool HasValidHandle(int deviceindex);
private:
	//the distance after the light is too dim
	float FalloffRange = 0.0f;
	float m_intesity = 10;
	glm::vec3 m_position = glm::vec3(0, 0, 0);

	glm::vec3 m_direction = glm::vec3(0, 1, 0);
	ELightType::Type m_type = ELightType::Limit;
	bool DoesShadow = true;
	int ShadowId = -1;
	bool ShadowNeedsUpdate[MAX_GPU_DEVICE_COUNT] = { true };
	ELightMobility::Type ShadowMode = ELightMobility::Realtime;
};

