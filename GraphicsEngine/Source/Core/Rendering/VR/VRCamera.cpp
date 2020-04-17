#include "VRCamera.h"
#include "RHI\RHI.h"
#include "Core\Transform.h"
#include "HMDManager.h"
#include "Core\Input\Input.h"


VRCamera::VRCamera()
{
	Cameras[EEye::Left] = new Camera();
	Cameras[EEye::Right] = new Camera();
	//debug
	transfrom.SetPos(glm::vec3(0, 10, 0));


	Cameras[EEye::Left]->fov = 10;
	Cameras[EEye::Right]->fov = 10;
}

VRCamera::~VRCamera()
{
}

void VRCamera::UpdateDebugTracking(float IPD)
{
	//transfrom.SetEulerRot(glm::vec3(0, 0, 45 + ((RHI::GetFrameCount() % 200)*0.1f)));
	//RHI::GetVrSettings()->EyeDistance = glm::max(-1.0f, RHI::GetVrSettings()->EyeDistance);
	Cameras[EEye::Left]->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());
	Cameras[EEye::Right]->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());

	glm::vec3 Pos = transfrom.GetPos();
	//Pos += transfrom.GetForward()*glm::vec3(-0.9, 0, -0.4);
	//Pos -= transfrom.GetRight() * IPD;
	Cameras[EEye::Left]->SetPos(Pos);

	Pos = transfrom.GetPos();
	//Pos += transfrom.GetRight() * IPD;
	//Pos += transfrom.GetForward()* glm::vec3(0.9, 0, -0.4);
	Cameras[EEye::Right]->SetPos(Pos);

	//if (Input::GetMouseButtonDown(0))
	//{
	//	transfrom.Translate(glm::vec3(0, 1, 0),1.0);
	//}
	//	CameraInstance->GetEyeCam(EEye::Left)->SetPos(glm::vec3(-0.9,0,-0.4));
	//CameraInstance->GetEyeCam(EEye::Left)->SetPos(glm::vec3(0.9, 0, -0.4));
}

Camera * VRCamera::GetEyeCam(EEye::Type type)
{
	return Cameras[type];
}

Transform * VRCamera::GetTransfrom()
{
	return &transfrom;
}
