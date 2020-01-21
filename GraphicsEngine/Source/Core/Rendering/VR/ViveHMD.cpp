#include "ViveHMD.h"
#include "OpenVR/headers/openvr.h"
#include "VRCamera.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Input/InputManager.h"
#include "Core/Input/Input.h"
#include "Core/Input/Interfaces/SteamVR/SteamVRInputInterface.h"
#include "HMDManager.h"
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
glm::mat4 ViveHMD::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{

	vr::HmdMatrix44_t mat = VRInterface->GetSystem()->GetProjectionMatrix(nEye, 0.1f, 1000);

	glm::mat4 out = glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);

	//out = glm::rowMajor4(out);
	return out;
}

glm::mat4 ViveHMD::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	vr::HmdMatrix34_t matEyeRight = VRInterface->GetSystem()->GetEyeToHeadTransform(nEye);
	glm::mat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);
	return glm::inverse(matrixObj);
}
glm::mat4 ConvertRHToLH(glm::mat4x4 RH)
{
	glm::mat4 LH = glm::mat4(
		RH[0][0], RH[2][0], RH[1][0], RH[3][0],
		RH[0][1], RH[2][1], RH[1][1], RH[3][1],
		RH[0][2], RH[2][2], RH[1][2], RH[3][2],
		RH[0][3], RH[2][3], RH[1][3], RH[3][3]);

	LH = glm::rowMajor4(LH);
	return LH;
}
glm::mat4 ViveHMD::getRaw(vr::Hmd_Eye nEye)
{
	float Left, Right, Top, Bottom;
	VRInterface->GetSystem()->GetProjectionRaw(nEye, &Right, &Left, &Top, &Bottom);
	glm::vec2 tanHalfFov = glm::vec2(glm::max(Left, Right), glm::max(Top, Bottom));
	float  fieldOfView =(2.0f * glm::atan(tanHalfFov.y));// *Mathf.Rad2Deg;
	float  aspect = tanHalfFov.x / tanHalfFov.y;
	glm::mat4 Mat = glm::perspectiveLH(fieldOfView, aspect, 0.1f, 1000.0f);
	return Mat;
}
float ViveHMD::GetIPD()
{
	vr::ETrackedPropertyError Error  = vr::TrackedProp_Success;
	float out = VRInterface->GetSystem()->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float,&Error);
	ensure(Error == vr::TrackedProp_Success);
	return out;
}

void ViveHMD::Update()
{
#if 1
	UpdateProjection((float)GetDimentions().x / (float)GetDimentions().y);
	HMD::Update();
	CameraInstance->UpdateDebugTracking(GetIPD());
#if 1
	CameraInstance->GetEyeCam(EEye::Left)->SetProjection(getRaw(vr::Eye_Left));
	CameraInstance->GetEyeCam(EEye::Right)->SetProjection(getRaw(vr::Eye_Right));
#endif
#else
	CameraInstance->GetEyeCam(EEye::Left)->SetProjection(ConvertRHToLH(GetHMDMatrixProjectionEye(vr::Eye_Left)));
	CameraInstance->GetEyeCam(EEye::Right)->SetProjection(ConvertRHToLH(GetHMDMatrixProjectionEye(vr::Eye_Right)));
	CameraInstance->GetEyeCam(EEye::Left)->SetViewTransFrom(ConvertRHToLH(GetHMDMatrixPoseEye(vr::Eye_Left))* glm::inverse(ConvertRHToLH(VRInterface->poses[vr::k_unTrackedDeviceIndex_Hmd])));

	CameraInstance->GetEyeCam(EEye::Right)->SetViewTransFrom(ConvertRHToLH(GetHMDMatrixPoseEye(vr::Eye_Right))* glm::inverse(ConvertRHToLH(VRInterface->poses[vr::k_unTrackedDeviceIndex_Hmd])));
#endif
}

void ViveHMD::OutputToEye(FrameBuffer* buffer, EEye::Type eye)
{
	RHI::SubmitToVRComposter(buffer, eye);
}

glm::ivec2 ViveHMD::GetDimentions()
{
	uint32_t Width, height;
	VRInterface->GetSystem()->GetRecommendedRenderTargetSize(&Width, &height);
	return glm::ivec2(Width, height);
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