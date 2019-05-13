#include "Stdafx.h"
#include "HMD.h"
#include "VRCamera.h"
#include "DebugHMD.h"


HMD::HMD()
{}


HMD::~HMD()
{}

HMD * HMD::Create()
{
	//if needed return different HMD class
	
	return new DebugHMD();
}

VRCamera * HMD::GetVRCamera()
{
	return CameraInstance;
}

void HMD::Init()
{
	CameraInstance = new VRCamera();

}

void HMD::Update()
{
	CameraInstance->UpdateTracking();
}

void HMD::OutputToEye(FrameBuffer * buffer, EEye::Type eye)
{}

void HMD::UpdateProjection(float aspect)
{
	CameraInstance->GetEyeCam(EEye::Left)->UpdateProjection(aspect);
	CameraInstance->GetEyeCam(EEye::Right)->UpdateProjection(aspect);
}

glm::ivec2 HMD::GetDimentions()
{
	return glm::ivec2(1080, 1200);
}
