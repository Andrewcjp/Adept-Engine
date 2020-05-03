
#include "EditorCameraController.h"
#include "Core\Input\Input.h"
#include "Core\Transform.h"
#include "Core\Input\InputManager.h"
#include "Core\Input\InputController.h"


EditorCameraController::EditorCameraController()
{
}


EditorCameraController::~EditorCameraController()
{
}


void EditorCameraController::TickController(float dt)
{
	if (Input::GetInputManager()->GetController(0) == nullptr)
	{
		return;
	}
	InputController* con = Input::GetInputManager()->GetController(0);
	glm::vec2 MoveAxis = con->GetLeftThumbStickAxis();
	float ZAxis = con->GetRightTriggerAxis() - con->GetLeftTriggerAxis();
	if (glm::length(MoveAxis) > 0.1 || abs(ZAxis) > 0.1)
	{
		float movespeed = BaseTranslateSpeed;
		if (con->GetButton(GamePadButtons::LeftShoulder))
		{
			movespeed = FastTranslateSpeed;
		}
		else
		{
			movespeed = BaseTranslateSpeed;
		}
		glm::vec3 WorldAxis = (Target->GetForward()*-MoveAxis.y) + (Target->GetRight()*-MoveAxis.x) + (Target->GetUp()*ZAxis);
		Target->Translate(WorldAxis, movespeed*dt);
	}


	glm::vec2 axis = con->GetRightThumbStickAxis();
	Rotx += axis.y * ControllerSens*dt;
	RotY += axis.x * ControllerSens*dt;
	ensure(!glm::isnan(Rotx));
	ensure(!glm::isnan(RotY));
	glm::quat rot = glm::quat();
	rot = glm::angleAxis(glm::radians(-RotY), glm::vec3(0, 1, 0));
	rot *= glm::angleAxis(glm::radians(Rotx), glm::vec3(1, 0, 0));
	Target->SetQrot(rot);
	MovedThisFrame = true;
}

void EditorCameraController::Update()
{
	const float dt = Engine::GetDeltaTime();
	TickController(dt);
#ifdef PLATFORM_WINDOWS
	if (Input::GetMouseButton(MouseButton::ButtonRight))
#endif
	{
		Input::LockCursor(true);
		Input::SetCursorVisible(false);
		float movespeed = BaseTranslateSpeed;
		if (Input::GetKey(KeyCode::Shift))
		{
			movespeed = FastTranslateSpeed;
		}
		else
		{
			movespeed = BaseTranslateSpeed;
		}
		if (Input::GetKey(KeyCode::W))
		{
			Target->Translate(Target->GetForward(), movespeed*dt);
		}
		if (Input::GetKey(KeyCode::S))
		{
			Target->Translate(Target->GetForward(), -movespeed * dt);
		}
		if (Input::GetKey(KeyCode::A))
		{
			Target->Translate(Target->GetRight(), -movespeed * dt);
		}
		if (Input::GetKey(KeyCode::D))
		{
			Target->Translate(Target->GetRight(), movespeed * dt);
		}
		if (Input::GetKey(KeyCode::E))
		{
			Target->Translate(Target->GetUp(), movespeed*dt);
		}
		if (Input::GetKey(KeyCode::Q))
		{
			Target->Translate(Target->GetUp(), -movespeed * dt);
		}
		glm::vec2 axis = Input::GetMouseInputAsAxis();

		Rotx += Input::GetMouseInputAsAxis().y* LookSens;
		RotY += Input::GetMouseInputAsAxis().x* LookSens;
		ensure(!glm::isnan(Rotx));
		ensure(!glm::isnan(RotY));
		glm::quat rot = glm::quat();
		rot = glm::angleAxis(glm::radians(-RotY), glm::vec3(0, 1, 0));
		rot *= glm::angleAxis(glm::radians(Rotx), glm::vec3(1, 0, 0));
		Target->SetQrot(rot);
		MovedThisFrame = true;
	}
#ifdef PLATFORM_WINDOWS
	else
	{
		MovedThisFrame = false;
		Input::LockCursor(false);
		Input::SetCursorVisible(true);
	}
#endif

}

bool EditorCameraController::IsMoving()
{
	return MovedThisFrame;
}
