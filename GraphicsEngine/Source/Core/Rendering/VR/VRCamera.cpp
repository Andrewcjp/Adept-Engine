#include "Stdafx.h"
#include "VRCamera.h"
#include "RHI\RHI.h"
#include "Core\Transform.h"
#include "HMDManager.h"


VRCamera::VRCamera()
{
	Cameras[EEye::Left] = new Camera();
	Cameras[EEye::Right] = new Camera();
	//debug
	transfrom.SetPos(glm::vec3(0, 10, 0));
}

VRCamera::~VRCamera()
{}

void VRCamera::UpdateDebugTracking()
{
	RHI::GetVrSettings()->EyeDistance = glm::max(0.0f, RHI::GetVrSettings()->EyeDistance);
	glm::vec3 Pos = transfrom.GetPos();
	Pos -= transfrom.GetRight() * RHI::GetVrSettings()->EyeDistance;
	Cameras[EEye::Left]->SetPos(Pos);
	Cameras[EEye::Left]->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());

	Pos = transfrom.GetPos();
	Pos += transfrom.GetRight() * RHI::GetVrSettings()->EyeDistance;
	Cameras[EEye::Right]->SetPos(Pos);
	Cameras[EEye::Right]->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());
}

Camera * VRCamera::GetEyeCam(EEye::Type type)
{
	return Cameras[type];
}

Transform * VRCamera::GetTransfrom()
{
	return &transfrom;
}
