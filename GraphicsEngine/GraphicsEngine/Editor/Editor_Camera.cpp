#include "stdafx.h"
#include "Editor_Camera.h"
#include "Core/Input.h"

Editor_Camera::Editor_Camera(Camera* cam)
{
	if (cam == nullptr)
	{
		MainCam = new Camera(glm::vec3(0, 10, 0), 75.0f, 1.77f, 0.1f, 1000.0f);
	}
	else
	{
		MainCam = cam;
	}
	IsActive = true;
	sensitvity = 1.0f / 1000.0f;
}


Editor_Camera::~Editor_Camera()
{
	delete MainCam;
}

void Editor_Camera::Update(float delatime)
{
	if (Input::GetVKey(VK_RBUTTON))
	{
		Input::instance->LockCursor(true);
		float movespeed = 100;
		if (Input::GetVKey(VK_LSHIFT))
		{
			movespeed = 1000;
		}
		else
		{
			movespeed = 100;
		}
		if (Input::GetKey('w'))
		{
			MainCam->MoveForward(movespeed*delatime);
		}
		if (Input::GetKey('s'))
		{
			MainCam->MoveForward(-movespeed*delatime);
		}
		if (Input::GetKey('a'))
		{
			MainCam->MoveRight(movespeed*delatime);
		}
		if (Input::GetKey('d'))
		{
			MainCam->MoveRight(-movespeed*delatime);
		}
		if (Input::GetKey('e'))
		{
			MainCam->MoveUp(movespeed*delatime);
		}
		if (Input::GetKey('q'))
		{
			MainCam->MoveUp(-movespeed*delatime);
		}
		glm::vec2 axis = Input::GetMouseInputAsAxis();
		MainCam->RotateY(axis.x*sensitvity);
		MainCam->Pitch(axis.y*sensitvity);
		
	}
	else
	{
		Input::instance->LockCursor(false);

	}
}
