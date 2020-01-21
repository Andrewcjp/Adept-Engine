#include "SteamVRInputInterface.h"
#if BUILD_STEAMVR
#include "Core/Utils/DebugDrawers.h"
#include "Rendering/VR/ViveHMD.h"
#include "Core/Assets/AssetManager.h"

SteamVRInputInterface::SteamVRInputInterface()
{
	Init();
}

SteamVRInputInterface::~SteamVRInputInterface()
{}

bool SteamVRInputInterface::CanInit()
{
	if (RHI::GetRenderSettings()->VRHMDMode != EVRHMDMode::SteamVR)
	{
		return false;
	}
	return vr::VR_IsHmdPresent();
}

void SteamVRInputInterface::Init()
{
	vr::HmdError error;
	system = vr::VR_Init(&error, vr::VRApplication_Scene);
	ensure(error == vr::HmdError::VRInitError_None);
	ensure(vr::VRCompositor());

	vr::VRInput()->SetActionManifestPath((AssetManager::GetContentPath() + "Mani.json").c_str());
}


InputController* SteamVRInputInterface::GetController(int Index)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int SteamVRInputInterface::GetNumOfControllers() const
{
	return 0;
}

bool SteamVRInputInterface::HasHMD() const
{
	return true;
}

void SteamVRInputInterface::ShutDown()
{
	vr::VR_Shutdown();
}

glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], matPose.m[3][0],
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	matrixObj = glm::rowMajor4(matrixObj);
	return matrixObj;
}

glm::mat4 ConvertSteamVRMatrixToMatrix4_T(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	matrixObj[0][2] = -matrixObj[0][2];
	matrixObj[1][2] = -matrixObj[1][2];

	matrixObj[2][0] = -matrixObj[2][0];
	matrixObj[2][1] = -matrixObj[2][1];
	matrixObj[2][3] = -matrixObj[2][3];

	//matrixObj = glm::rowMajor4(matrixObj);
	return matrixObj;
}

glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix44_t &mat)
{
	glm::mat4 matrixObj(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
	matrixObj = glm::rowMajor4(matrixObj);
	return matrixObj;
}

static float _copysign(float sizeval, float signval)
{
	return glm::sign(signval) == 1 ? glm::abs(sizeval) : -glm::abs(sizeval);
}

static glm::quat GetRotation(glm::mat4 matrix)
{
	glm::quat q;// = new Quaternion();
	q.w = glm::sqrt(glm::max(0.0f, 1.0f + matrix[0][0] + matrix[1][1] + matrix[2][2])) / 2.0f;
	q.x = glm::sqrt(glm::max(0.0f, 1.0f + matrix[0][0] - matrix[1][1] - matrix[2][2])) / 2.0f;
	q.y = glm::sqrt(glm::max(0.0f, 1.0f - matrix[0][0] + matrix[1][1] - matrix[2][2])) / 2.0f;
	q.z = glm::sqrt(glm::max(0.0f, 1.0f - matrix[0][0] - matrix[1][1] + matrix[2][2])) / 2.0f;
	q.x = _copysign(q.x, matrix[2][1] - matrix[1][2]);
	q.y = _copysign(q.y, matrix[0][2] - matrix[2][0]);
	q.z = _copysign(q.z, matrix[1][0] - matrix[0][1]);
	return q;
}

glm::vec3 SteamVRInputInterface::Getpos(glm::mat4 matMVP)
{
	return glm::vec3(matMVP[0][3], matMVP[1][3], -matMVP[2][3])+Offset;
}

vr::IVRSystem * SteamVRInputInterface::GetSystem()
{
	return system;
}

void RenderTransfrom(Transform t)
{
	glm::vec3 pos = t.GetPos();
	DebugDrawers::DrawDebugLine(pos, pos + t.GetUp(), glm::vec3(1, 0, 0));
	DebugDrawers::DrawDebugLine(pos, pos + t.GetRight(), glm::vec3(0, 1, 0));
	DebugDrawers::DrawDebugLine(pos, pos + t.GetForward(), glm::vec3(0, 0, 1));
}

void SteamVRInputInterface::Tick()
{
	if (HMD == nullptr)
	{
		return;
	}
	//vr::VRInput()->UpdateActionState(,);
	//vr::VRActionHandle_t T;
	//vr::VRInput()->GetActionHandle("Test", &T);
	//
	//vr::VRInput()->GetDigitalActionData(T,)
	//	system->PollNextEventWithPose()
	vr::VREvent_t event;
	while (system->PollNextEvent(&event, sizeof(event)))
	{
		//ProcessVREvent(event);
	}
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		vr::VRControllerState_t state;
		if (system->GetControllerState(unDevice, &state, sizeof(state)))
		{
			//m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;
		}
	}
	vr::VRCompositor()->SetTrackingSpace(vr::ETrackingUniverseOrigin::TrackingUniverseSeated);
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			poses[nDevice] = ConvertSteamVRMatrixToMatrix4_T(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (system->GetTrackedDeviceClass(nDevice) == vr::TrackedDeviceClass_HMD)
			{
				HMD->SetPosAndRot(Getpos(ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking)), GetRotation(glm::inverse(poses[nDevice])));
			}
			else
			{
				TransArray[nDevice].SetPos(Getpos(ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking)));
				TransArray[nDevice].SetQrot(GetRotation(glm::inverse(poses[nDevice])));
				RenderTransfrom(TransArray[nDevice]);
				if (system->GetTrackedDeviceClass(nDevice) == vr::TrackedDeviceClass_Controller)
				{
					/*if (RHI::GetFrameCount() % 60 == 0)
					{
						vr::VRSystem()->TriggerHapticPulse(nDevice, 0, 100);
					}*/
				}
			}
		}
	}
}
#endif