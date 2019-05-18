#pragma once
#include "HMD.h"
#include "..\headers\openvr.h"
namespace vr
{
	class IVRSystem;
}
class ViveHMD: public HMD

{
public:
	ViveHMD();
	virtual ~ViveHMD();
	static bool CanCreate();
	void Init();

	virtual void Update() override;
	virtual void OutputToEye(FrameBuffer* buffer, EEye::Type eye) override;
	virtual glm::ivec2 GetDimentions() override;


	virtual bool IsActive() override;

private:
	glm::mat4 poses[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 HMDPose;
	vr::IVRSystem* system;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
};

