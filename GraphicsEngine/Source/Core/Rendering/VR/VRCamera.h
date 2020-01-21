#pragma once
#include "HMD.h"

class VRCamera
{
public:
	VRCamera();
	~VRCamera();
	void UpdateDebugTracking(float IPD);
	Camera* GetEyeCam(EEye::Type type);
	Transform* GetTransfrom();

private:
	Camera* Cameras[EEye::Limit] = { nullptr,nullptr };
	Transform transfrom;
};

