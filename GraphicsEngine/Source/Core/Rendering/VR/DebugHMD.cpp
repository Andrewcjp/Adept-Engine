#include "Stdafx.h"
#include "DebugHMD.h"
#include "Core\BaseWindow.h"
#include "VRCamera.h"
#include "Core\Input\Input.h"


DebugHMD::DebugHMD()
{}


DebugHMD::~DebugHMD()
{}

void DebugHMD::Update()
{
	//copy the main cam
	Camera* maincam = BaseWindow::GetCurrentCamera();
	CameraInstance->GetTransfrom()->SetPos(maincam->GetPosition());
	if (Input::GetMouseButtonDown(1))
	{
		glm::vec3 startrot = CameraInstance->GetTransfrom()->GetEulerRot();
		Log::LogTextToScreen(glm::to_string(startrot));
		float speed = 0.1f;
		roty += Input::GetMouseInputAsAxis().x* speed;
		startrot.y = -roty;
		startrot.x += Input::GetMouseInputAsAxis().y*speed;
		startrot.z = 0;

		//CameraInstance->GetTransfrom()->SetQrot(glm::quat(glm::radians(startrot)));
		CameraInstance->GetTransfrom()->SetEulerRot(startrot);
	}
	HMD::Update();
}
