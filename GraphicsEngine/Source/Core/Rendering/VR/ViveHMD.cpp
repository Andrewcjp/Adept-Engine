#include "Stdafx.h"
#include "ViveHMD.h"
#include "OpenVR/headers/openvr.h"
using namespace vr;
ViveHMD::ViveHMD()
{}


ViveHMD::~ViveHMD()
{
	VR_Shutdown();
}

bool ViveHMD::CanCreate()
{
	return VR_IsHmdPresent();
}

void ViveHMD::Init()
{
	HmdError error;
	system = VR_Init(&error, VRApplication_Scene);
	ensure(error == HmdError::VRInitError_None);
}

void ViveHMD::Update()
{

}

void ViveHMD::OutputToEye(FrameBuffer* buffer, EEye::Type eye)
{
	RHI::SubmitToVRComposter(buffer, eye);
}

glm::ivec2 ViveHMD::GetDimentions()
{
	uint32_t Width, height;
	system->GetRecommendedRenderTargetSize(&Width, &height);
	return glm::ivec2(Width, height);
}

bool ViveHMD::IsActive()
{
	return true;
}
