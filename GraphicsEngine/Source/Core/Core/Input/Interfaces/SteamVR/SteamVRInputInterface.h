#pragma once
#include "../../InputInterface.h"
#include "..\headers\openvr.h"

class ViveHMD;
//handles the VRsystem
//HMD updates itself from this system
//controllers handled as objects
class SteamVRInputInterface :public InputInterface
{
public:
	SteamVRInputInterface();
	~SteamVRInputInterface();
	static bool CanInit();
	void Init();
	virtual void Tick() override;
	virtual InputController* GetController(int Index) override;
	virtual int GetNumOfControllers() const override;
	virtual bool HasHMD() const override;

	virtual void ShutDown() override;

	glm::vec3 Getpos(glm::mat4 matMVP);
	vr::IVRSystem* GetSystem();
	ViveHMD* HMD = nullptr;
private:
	
	glm::mat4 poses[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 HMDPose;
	glm::quat Rot;
	vr::IVRSystem* system;
	glm::mat4 posm;
	Transform TransArray[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	float RenderScale = 1.0f;
	glm::vec3 Offset = glm::vec3(0, 5, 0);
	glm::vec3 Scale = glm::vec3(10);
};

