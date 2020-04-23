#include "ViveHMD.h"
#include "OpenVR/headers/openvr.h"
#include "VRCamera.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Input/InputManager.h"
#include "Core/Input/Input.h"
#include "Core/Input/Interfaces/SteamVR/SteamVRInputInterface.h"
#include "HMDManager.h"
#include <directxmath.h>
#pragma optimize("",off)
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
const float Rad2Deg = 57.29578F;
float ProjectFOV = 0;
glm::mat4 ViveHMD::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{

	vr::HmdMatrix44_t mat = VRInterface->GetSystem()->GetProjectionMatrix(nEye, 0.1f, 1000);

	glm::mat4 out = glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
	float fov = glm::atan(1.0f / out[1][1]) * 2 * Rad2Deg;
	ProjectFOV = fov;
	out = glm::inverse(out);
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

float  aspectr;
glm::mat4 ViveHMD::getRaw(vr::Hmd_Eye nEye)
{
	//float Left, Right, Top, Bottom;
	//VRInterface->GetSystem()->GetProjectionRaw(nEye, &Right, &Left, &Top, &Bottom);
	glm::vec2 tanHalfFov/* = glm::vec2(glm::max(Left, Right), glm::max(Top, Bottom))*/;


	float l_left = 0.0f, l_right = 0.0f, l_top = 0.0f, l_bottom = 0.0f;
	VRInterface->GetSystem()->GetProjectionRaw(vr::Eye_Left, &l_left, &l_right, &l_top, &l_bottom);

	float r_left = 0.0f, r_right = 0.0f, r_top = 0.0f, r_bottom = 0.0f;
	VRInterface->GetSystem()->GetProjectionRaw(vr::Eye_Right, &r_left, &r_right, &r_top, &r_bottom);

	tanHalfFov = glm::vec2(
		glm::max(glm::max(-l_left, l_right), glm::max(-r_left, r_right)),
		glm::max(glm::max(-l_top, l_bottom), glm::max(-r_top, r_bottom)));
	uint32_t Width, height;
	VRInterface->GetSystem()->GetRecommendedRenderTargetSize(&Width, &height);
	sceneWidth = (float)Width;
	sceneHeight = (float)height;
	vr::VRTextureBounds_t textureBounds[2];

	textureBounds[0].uMin = 0.5f + 0.5f * l_left / tanHalfFov.x;
	textureBounds[0].uMax = 0.5f + 0.5f * l_right / tanHalfFov.x;
	textureBounds[0].vMin = 0.5f - 0.5f * l_bottom / tanHalfFov.y;
	textureBounds[0].vMax = 0.5f - 0.5f * l_top / tanHalfFov.y;

	textureBounds[1].uMin = 0.5f + 0.5f * r_left / tanHalfFov.x;
	textureBounds[1].uMax = 0.5f + 0.5f * r_right / tanHalfFov.x;
	textureBounds[1].vMin = 0.5f - 0.5f * r_bottom / tanHalfFov.y;
	textureBounds[1].vMax = 0.5f - 0.5f * r_top / tanHalfFov.y;

	// Grow the recommended size to account for the overlapping fov
	sceneWidth = sceneWidth / glm::max(textureBounds[0].uMax - textureBounds[0].uMin, textureBounds[1].uMax - textureBounds[1].uMin);
	sceneHeight = sceneHeight / glm::max(textureBounds[0].vMax - textureBounds[0].vMin, textureBounds[1].vMax - textureBounds[1].vMin);


	float  fieldOfView = (2.0f * glm::atan(tanHalfFov.y));// *Mathf.Rad2Deg;
	float FOVDEG = fieldOfView * Rad2Deg;
	//111.6932
	float  aspect = tanHalfFov.x / tanHalfFov.y;
	aspectr = aspect;
	//aspect 0.947466 FOV:111.6932

	//aspect = 0.947466;
	aspect = ((float)Width / (float)height);
	if (Input::GetKey(' '))
	{
		fieldOfView = fieldOfView;
	}
	fieldOfView = ProjectFOV / Rad2Deg;
	glm::mat4 Mat = glm::perspectiveLH(fieldOfView, aspect, 0.1f, 1000.0f);
	return Mat;
}
float ViveHMD::GetIPD()
{
	vr::ETrackedPropertyError Error = vr::TrackedProp_Success;
	float out = VRInterface->GetSystem()->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float, &Error);
	ensure(Error == vr::TrackedProp_Success);
	return out;
}
glm::mat4 ViveHMD::GetOffCentreRaw(vr::Hmd_Eye eye)
{
	float l_left = 0.0f, l_right = 0.0f, l_top = 0.0f, l_bottom = 0.0f;
	VRInterface->GetSystem()->GetProjectionRaw(eye, &l_left, &l_right, &l_top, &l_bottom);


	DirectX::XMMATRIX  output = DirectX::XMMatrixPerspectiveOffCenterLH(l_left, l_right, l_bottom, l_top, 0.1, 1000.0f);
	glm::mat4 matrixObj(
		output.r[0].m128_f32[0], output.r[0].m128_f32[1], output.r[0].m128_f32[2], output.r[0].m128_f32[3],
		output.r[1].m128_f32[0], output.r[1].m128_f32[1], output.r[1].m128_f32[2], output.r[1].m128_f32[3],
		output.r[2].m128_f32[0], output.r[2].m128_f32[1], output.r[2].m128_f32[2], output.r[2].m128_f32[3],
		output.r[3].m128_f32[0], output.r[3].m128_f32[1], output.r[3].m128_f32[2], output.r[3].m128_f32[3]
	);
	matrixObj = glm::rowMajor4(matrixObj);


	const vr::HmdMatrix44_t vrHmdMatrix34 = VRInterface->GetSystem()->GetProjectionMatrix(static_cast<vr::Hmd_Eye>(eye), 0.1f, 1000.0f);
	return glm::mat4(
		vrHmdMatrix34.m[0][0], vrHmdMatrix34.m[1][0], vrHmdMatrix34.m[2][0], vrHmdMatrix34.m[3][0],
		vrHmdMatrix34.m[0][1], vrHmdMatrix34.m[1][1], vrHmdMatrix34.m[2][1], vrHmdMatrix34.m[3][1],
		-vrHmdMatrix34.m[0][2], -vrHmdMatrix34.m[1][2], -vrHmdMatrix34.m[2][2], -vrHmdMatrix34.m[3][2],
		vrHmdMatrix34.m[0][3], vrHmdMatrix34.m[1][3], vrHmdMatrix34.m[2][3], vrHmdMatrix34.m[3][3]
	);
	return matrixObj;

}
void ViveHMD::Update()
{
#if 1
	UpdateProjection(aspectr);
	HMD::Update();
	CameraInstance->UpdateDebugTracking(GetIPD());
#if 1
#if 1
	GetHMDMatrixProjectionEye(vr::Eye_Right);
	glm::mat4 proj = getRaw(vr::Eye_Left);
	CameraInstance->GetEyeCam(EEye::Left)->SetProjection(GetHMDMatrixPoseEye(vr::Eye_Left)*proj);
	CameraInstance->GetEyeCam(EEye::Right)->SetProjection(GetHMDMatrixPoseEye(vr::Eye_Right)*proj);
#else
	CameraInstance->GetEyeCam(EEye::Left)->SetProjection(GetHMDMatrixPoseEye(vr::Eye_Left)*GetHMDMatrixProjectionEye(vr::Eye_Left));
	CameraInstance->GetEyeCam(EEye::Right)->SetProjection(GetHMDMatrixPoseEye(vr::Eye_Right)*GetHMDMatrixProjectionEye(vr::Eye_Right));
#endif
	//VRInterface->GetSystem()->GetEyeToHeadTransform(vr::Eye_Left);


	//CameraInstance->GetEyeCam(EEye::Left)->SetUpAndForward(transfrom.GetForward(), transfrom.GetUp());
#endif
#else
	CameraInstance->GetEyeCam(EEye::Left)->SetProjection(ConvertRHToLH(GetHMDMatrixProjectionEye(vr::Eye_Left)));
	CameraInstance->GetEyeCam(EEye::Right)->SetProjection(ConvertRHToLH(GetHMDMatrixProjectionEye(vr::Eye_Right)));
	CameraInstance->GetEyeCam(EEye::Left)->SetViewTransFrom(ConvertRHToLH(GetHMDMatrixPoseEye(vr::Eye_Left))* glm::inverse(ConvertRHToLH(VRInterface->poses[vr::k_unTrackedDeviceIndex_Hmd])));

	CameraInstance->GetEyeCam(EEye::Right)->SetViewTransFrom(ConvertRHToLH(GetHMDMatrixPoseEye(vr::Eye_Right))* glm::inverse(ConvertRHToLH(VRInterface->poses[vr::k_unTrackedDeviceIndex_Hmd])));
#endif

	//HiddenAreaMesh_t GetHiddenAreaMesh( Hmd_Eye eEye )
}

void ViveHMD::OutputToEye(FrameBuffer* buffer, EEye::Type eye)
{
	RHI::SubmitToVRComposter(buffer, eye);
}

glm::ivec2 ViveHMD::GetDimentions()
{
	uint32_t Width, height;
	VRInterface->GetSystem()->GetRecommendedRenderTargetSize(&Width, &height);
	getRaw(vr::Eye_Left);

	return glm::ivec2(Width, sceneHeight);
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