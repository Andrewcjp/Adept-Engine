#include "stdafx.h"
#include "Editor_Camera.h"
#include "../Core/Input.h"

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
	float movespeed = 100;
	if (GetKeyState(VK_LSHIFT) & 0x8000)
	{
		// Right ALT key is down.
		movespeed = 1000;
	}
	else
	{
		movespeed = 100;
	}
	if (GetKeyState(87) & 0x8000)
	{
		// Right ALT key is down.
		MainCam->MoveForward(movespeed*delatime);
	}
	if (GetKeyState(83) & 0x8000)
	{
		// Right ALT key is down.
		MainCam->MoveForward(-movespeed*delatime);
	}
	if (GetKeyState(65) & 0x8000)
	{
		// Right ALT key is down.
		MainCam->MoveRight(movespeed*delatime);
	}
	if (GetKeyState(68) & 0x8000)
	{
		// Right ALT key is down.
		MainCam->MoveRight(-movespeed*delatime);
	}
	if (GetKeyState(69) & 0x8000)
	{
		// Right ALT key is down.
		MainCam->MoveUp(movespeed*delatime);
	}
	if (GetKeyState(81) & 0x8000)
	{
		// Right ALT key is down.
		MainCam->MoveUp(-movespeed*delatime);
	}
	glm::vec2 axis = Input::GetMouseInputAsAxis();
	MainCam->RotateY(axis.x*sensitvity);
	MainCam->Pitch(axis.y*sensitvity);
}
