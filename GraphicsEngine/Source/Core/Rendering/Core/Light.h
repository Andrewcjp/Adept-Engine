#pragma once
#include "Core/Transform.h"


class Light
{
public:
	static int LastId;
	enum LightType
	{
		Directional, Point, Spot,Area
	};
	Light(glm::vec3 positon, float intesity, LightType type = LightType::Point, glm::vec3 Lightcolor = glm::vec3(1, 1, 1), bool shadow = true);
	~Light();
	glm::vec3 GetPosition() const;
	glm::vec3 GetColor() const;
	LightType GetType() const;
	void SetPostion(glm::vec3 pos);
	void SetIntesity(float value);
	void SetLightType(LightType value);
	void SetShadow(bool state);
	void SetDirection(const glm::vec3 &value);
	bool GetDoesShadow() const;
	void SetShadowId(int id);
	int GetShadowId() const;
	int DirectionalShadowid = -1;
	glm::vec3 GetDirection()const;
	float GetIntesity();
	glm::mat4 DirView;
	glm::mat4 Projection;
	glm::vec3 m_lightColor = glm::vec3(1, 1, 1);
	//Multi GPU Data
	//Should the shadow be sampled on this card or copied
	bool GPUShadowResidentMask[MAX_GPU_DEVICE_COUNT] = { true };
	//If a shadow is resident on this device which one should we copy it too?
	int GPUShadowCopyDeviceTarget[MAX_GPU_DEVICE_COUNT] = { -1 ,-1};
	void SetShadowResdent(int DeviceIndex, int CopyTarget);
	float Distance = 512;
	void Update();
	float GetRange();
private:
	//the distance after the light is too dim
	float FalloffRange = 0.0f;
	float m_intesity = 10;
	glm::vec3 m_position = glm::vec3(0, 0, 0);

	glm::vec3 m_direction = glm::vec3(0, 1, 0);
	LightType m_type;
	bool DoesShadow = true;
	int ShadowId = -1;

};

