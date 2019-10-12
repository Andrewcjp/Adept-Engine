#include "ViveHMD.h"
#include "OpenVR/headers/openvr.h"
#include "VRCamera.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Input/InputManager.h"
#include "Core/Input/Input.h"
#include "Core/Input/Interfaces/SteamVR/SteamVRInputInterface.h"
#if BUILD_STEAMVR
ViveHMD::ViveHMD()
{
	RenderScale = 1.0f;
}

ViveHMD::~ViveHMD()
{
	vr::VR_Shutdown();
}

bool ViveHMD::CanCreate()
{
	return vr::VR_IsHmdPresent();
}

void ViveHMD::Init()
{
	HMD::Init();
	VRInterface = (SteamVRInputInterface*)Input::GetInputManager()->GetActiveVRInterface();
	VRInterface->HMD = this;
}

void ViveHMD::Update()
{
	UpdateProjection((float)GetDimentions().x / (float)GetDimentions().y);
	HMD::Update();
	CameraInstance->UpdateDebugTracking();
}

void ViveHMD::OutputToEye(FrameBuffer* buffer, EEye::Type eye)
{
	RHI::SubmitToVRComposter(buffer, eye);
}

glm::ivec2 ViveHMD::GetDimentions()
{
	uint32_t Width, height;
	VRInterface->GetSystem()->GetRecommendedRenderTargetSize(&Width, &height);
	return glm::ivec2(glm::iround(Width*RenderScale), glm::iround(height*RenderScale));
}

void ViveHMD::SetPosAndRot(glm::vec3 pos, glm::quat Rot)
{
	CameraInstance->GetTransfrom()->SetPos(pos);
	CameraInstance->GetTransfrom()->SetQrot(Rot);
}

bool ViveHMD::IsActive()
{
	return true;
}
#endif