#include "Stdafx.h"
#include "EditorCameraController.h"
#include "Core\Input\Input.h"


EditorCameraController::EditorCameraController()
{}


EditorCameraController::~EditorCameraController()
{}

void EditorCameraController::Update()
{
	const float dt = Engine::GetDeltaTime();
	if (Input::GetVKey(0x02))
	{
		Input::LockCursor(true);
		Input::SetCursorVisible(false);
		float movespeed = BaseTranslateSpeed;
		if (Input::GetVKey(0xA0))
		{
			movespeed = FastTranslateSpeed;
		}
		else
		{
			movespeed = BaseTranslateSpeed;
		}
#if 1
		if (Input::GetKey('w'))
		{
			Target->Translate(Target->GetForward(), movespeed*dt);
		}
		if (Input::GetKey('s'))
		{
			Target->Translate(Target->GetForward(), -movespeed * dt);
		}
		if (Input::GetKey('a'))
		{
			Target->Translate(Target->GetRight(), -movespeed*dt);
		}
		if (Input::GetKey('d'))
		{
			Target->Translate(Target->GetRight(), movespeed * dt);
		}
		if (Input::GetKey('e'))
		{
			Target->Translate(Target->GetUp(), movespeed*dt);
		}
		if (Input::GetKey('q'))
		{
			Target->Translate(Target->GetUp(), -movespeed * dt);
		}
#endif
		glm::vec2 axis = Input::GetMouseInputAsAxis();

		Rotx += Input::GetMouseInputAsAxis().y* LookSens;
		RotY += Input::GetMouseInputAsAxis().x* LookSens;
		glm::quat rot = glm::quat();
		rot = glm::angleAxis(glm::radians(-RotY), glm::vec3(0, 1, 0));
		rot *= glm::angleAxis(glm::radians(Rotx), glm::vec3(1, 0, 0));
		Target->SetQrot(rot);
	}
	else
	{
		Input::LockCursor(false);
		Input::SetCursorVisible(true);
	}

}
