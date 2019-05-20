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


	Cameras[EEye::Left]->fov = 110;
	Cameras[EEye::Right]->fov = 110;
}

VRCamera::~VRCamera()
{}

void VRCamera::UpdateDebugTracking()
{
	float aspect = 1080.0f / 1200.0f;
	RHI::GetVrSettings()->EyeDistance = glm::max(-1.0f, RHI::GetVrSettings()->EyeDistance);

	glm::vec3 Pos = transfrom.GetPos();
	Pos -= transfrom.GetRight() * RHI::GetVrSettings()->EyeDistance;
	Cameras[EEye::Left]->SetPos(Pos);
	Cameras[EEye::Left]->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());
	Cameras[EEye::Left]->UpdateProjection(aspect);

	Pos = transfrom.GetPos();
	Pos += transfrom.GetRight() * RHI::GetVrSettings()->EyeDistance;
	Cameras[EEye::Right]->SetPos(Pos);
	Cameras[EEye::Right]->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());
	Cameras[EEye::Right]->UpdateProjection(aspect);
}

Camera * VRCamera::GetEyeCam(EEye::Type type)
{
	return Cameras[type];
}

Transform * VRCamera::GetTransfrom()
{
	return &transfrom;
}
