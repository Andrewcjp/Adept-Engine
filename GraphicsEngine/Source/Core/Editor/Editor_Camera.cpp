
#include "Editor_Camera.h"
#include "Core/Input/Input.h"
#include "Editor/EditorWindow.h"
#include "EditorCameraController.h"
#include "Rendering/VR/VRCamera.h"
#if WITH_EDITOR
Editor_Camera::Editor_Camera(Camera* cam)
{
	if (cam == nullptr)
	{
		float aspect = (float)EditorWindow::GetWidth() / (float)EditorWindow::GetHeight();
		MainCam = new Camera(glm::vec3(0, 10, 0), 75.0f, aspect, 0.1f, 1000.0f);
	}
	else
	{
		MainCam = cam;
	}
	MainCamTransfrom.SetPos(glm::vec3(0, 10, 0));
	IsActive = true;
	sensitvity = 1.0f / 1000.0f;
	Controller = new EditorCameraController();
}


Editor_Camera::~Editor_Camera()
{
	SafeDelete(MainCam);
}

void Editor_Camera::Update(float delatime)
{
	if (RHI::IsRenderingVR())
	{
		Controller->Target = RHI::GetHMD()->GetVRCamera()->GetTransfrom();
	}
	else
	{
		Controller->Target = &MainCamTransfrom;
	}
	Controller->Update();

	MainCam->Sync(&MainCamTransfrom);
}

Camera * Editor_Camera::GetCamera()
{
	if (RHI::IsRenderingVR())
	{
		return RHI::GetHMD()->GetVRCamera()->GetEyeCam(EEye::Left);
	}
	return MainCam;
}

void Editor_Camera::SetVrCam(bool state)
{
	bIsVRCam = state;
}

bool Editor_Camera::IsVRCam() const
{
	return bIsVRCam;
}
#endif