#include "Input.h"
#include "OpenGL\OGLWindow.h"
#include "RHI/RenderWindow.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/RigidBody.h"
#include "Core/Engine.h"
#include "include\glm\glm.hpp"
#include "Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Shaders/ShaderOutput.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Editor\EditorWindow.h"
#include "Components\MeshRendererComponent.h"
Input* Input::instance = nullptr;
HCURSOR Input::Cursor = NULL;
Input::Input(Camera* c, GameObject* playergo, HWND window, RenderWindow* wind)
{
	instance = this;
	playerGO = playergo;
	MainCam = c;
	m_hwnd = window;
	ogwindow = wind;
	OpenGlwindow = reinterpret_cast<EditorWindow*>(wind);

}


Input::~Input()
{
	DestroyCursor(Cursor);
}
void Input::CreateStackAtPoint()
{
	RayHit hit;
	if (Engine::PhysEngine->RayCastScene(MainCam->GetPosition(), MainCam->GetForward(), 100, &hit) == false)
	{
		return;
	}
	std::vector<RigidBody*> objs = Engine::PhysEngine->createStack(hit.position, 5, 0.5);

	for (size_t i = 0; i < objs.size(); i++)
	{
		GameObject* go = new GameObject();
		Material* mat = new Material(new OGLTexture("../asset/texture/bricks2.jpg"));

		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/cubeuv.obj", nullptr), mat));
		/*go->SetMaterial(mat);
		go->SetMesh(new OGLMesh(L"../asset/models/cubeuv.obj"));*/

		go->GetTransform()->SetPos(objs[i]->GetPosition());
		float scale = 0.5;
		go->GetTransform()->SetScale(glm::vec3(scale));

		go->actor = objs[i];
		ogwindow->AddPhysObj(go);
	}
}

void Input::FireAtScene()
{
	GameObject* go = new GameObject();
	Material* mat = new Material(new OGLTexture("../asset/texture/bricks2.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("../asset/models/TherealSpherer.obj", nullptr), mat));
	go->GetTransform()->SetPos(MainCam->GetPosition());
	float scale = 0.5;
	go->GetTransform()->SetScale(glm::vec3(scale));

	go->actor = Engine::PhysEngine->FirePrimitiveAtScene(MainCam->GetPosition() + MainCam->GetForward() * 2, MainCam->GetForward() * CurrentForce, scale);
	ogwindow->AddPhysObj(go);

}
void Input::Clear()
{
	KeyMap.clear();
	IsTidleDown = false;
}
void Input::ProcessInput(const float delatime)
{

	if (EditorWindow::CurrentContext != nullptr)
	{
		return;//block input!
	}
	if (FreeCamMode)
	{
		if (GetKeyState(VK_CONTROL) & 0x8000)
		{
			//if (GetKeyState('S') & 0x8000)
			//{
			////	__debugbreak();
			//}
			return;
		}
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
	}
}

BOOL Input::MouseLBDown(int x, int y)
{
	MouseLookActive = true;
	ShowCursor(false);
	return TRUE;
}

BOOL Input::MouseLBUp(int x, int y)
{
	MouseLookActive = false;
	ShowCursor(true);
	return TRUE;
}
void GetDesktopResolution(int& horizontal, int& vertical, HWND window)
{
	RECT desktop;
	// Get the size of screen to the variable desktop
	//	GetWindowRect(hDesktop, &desktop);
	GetClientRect(window, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right - desktop.left;
	vertical = desktop.bottom - desktop.top;
}
BOOL Input::MouseMove(int x, int y, double deltatime)
{
	if (MouseLookActive)
	{
		float sens = 1 / 1000.0f;
		int height, width = 0;
		GetDesktopResolution(height, width, m_hwnd);
		int halfheight = (height / 2);
		int halfwidth = (width / 2);

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(m_hwnd, &pt);
		MainCam->RotateY(((halfheight)-pt.x)*sens);
		MainCam->Pitch((-((halfwidth)-pt.y)*sens));
		pt.x = halfheight;
		pt.y = halfwidth;
		ClientToScreen(m_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	return TRUE;
}
void Input::SetSelectedObject(int index)
{
	instance->currentObjectIndex = index;
	if (instance->currentObjectIndex >= instance->ogwindow->GetCurrentRenderer()->GetObjects().size())
	{
		instance->currentObjectIndex = 0;
	}
	instance->Selectedobject = instance->ogwindow->GetCurrentRenderer()->GetObjects()[instance->currentObjectIndex];
}
void Input::ProcessQue()
{
	for (size_t i = 0; i < Inputque.size(); i++)
	{
		Inputque.front()->Func(this);
		Inputque.pop();
	}
}
BOOL Input::ProcessKeyDown(WPARAM key)
{

	InputEvent* ie = new InputEvent(this);
	switch (key)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case VK_F1:
		if (OpenGlwindow != nullptr)
		{
			if (OpenGlwindow->ShowHud)
			{
				OpenGlwindow->ShowHud = false;
			}
			else
			{
				OpenGlwindow->ShowHud = true;
			}
		}
		break;
	case VK_F2:
		if (OpenGlwindow != nullptr)
		{
			if (OpenGlwindow->ExtendedPerformanceStats)
			{
				OpenGlwindow->ExtendedPerformanceStats = false;
			}
			else
			{
				OpenGlwindow->ExtendedPerformanceStats = true;
			}
		}

		break;
	case VK_F3:
		if (OpenGlwindow != nullptr)
		{
			if (OpenGlwindow->ShowText)
			{
				OpenGlwindow->ShowText = false;
			}
			else
			{
				OpenGlwindow->ShowText = true;
			}
		}
		break;
	case VK_F4:

		if (FxAA)
		{
			FxAA = false;
		}
		else
		{
			FxAA = true;
		}
		if (Filters != nullptr)
		{
			Filters->SetFXAA(FxAA);
		}

		break;
	case VK_F5:
		if (main != nullptr)
		{
			ie->Func = &(Test);
			Inputque.push(ie);
		}
		break;
	case VK_F6:
		if (main != nullptr)
		{
			main->SetShadowVis();
		}
		break;
	case VK_F7:
		if (main != nullptr)
		{
			main->SetFullBright();
		}
		break;
	case VK_TAB:
		break;
	case VK_SPACE:
		FireAtScene();
		break;
	case VK_DOWN:
		CreateStackAtPoint();
		break;
	case VK_LEFT:
		CurrentForce -= 1000;
		break;
	case VK_NUMPAD0:
		currentObjectIndex++;
		if (currentObjectIndex >= ogwindow->GetCurrentRenderer()->GetObjects().size())
		{
			currentObjectIndex = 0;
		}
		Selectedobject = ogwindow->GetCurrentRenderer()->GetObjects()[currentObjectIndex];
		break;
	case VK_DECIMAL:
		currentObjectIndex--;
		if (currentObjectIndex < 0)
		{
			currentObjectIndex = 0;
		}
		Selectedobject = ogwindow->GetCurrentRenderer()->GetObjects()[currentObjectIndex];
		break;
	case VK_NUMPAD4:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(currentmoveamt, 0, 0));
		}
		break;
	case VK_NUMPAD6:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(-currentmoveamt, 0, 0));
		}
		break;
	case VK_NUMPAD8:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, 0, currentmoveamt));
		}
		break;
	case VK_NUMPAD5:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, 0, -currentmoveamt));
		}
		break;
	case VK_NUMPAD7:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, currentmoveamt, 0));
		}
		break;
	case VK_NUMPAD9:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, -currentmoveamt, 0));
		}
		break;
	case VK_RIGHT:
		CurrentForce += 1000;
		if (CurrentForce < 1000)
		{
			CurrentForce = 1000;
		}
		break;
	default:
		char c = MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		switch (c)
		{
		case '`'://w
			IsTidleDown = true;
			break;
		case 1:
			break;
		}
		KeyMap.emplace(c, true);
		return TRUE;
		break;
	}
	KeyMap.emplace((int)key, true);
	return TRUE;
}

void Input::Test(Input * in)
{
	in->main->SetNormalVis();
}

bool Input::GetKeyDown(int c)
{
	if (instance != nullptr)
	{
		if (instance->KeyMap.find((int)c) != instance->KeyMap.end())
		{
			return instance->KeyMap.at((int)c);
		}
		//if (GetKeyState(c) & 0x8000)
		//{
		//	// Right ALT key is down.
		//	return true;
		//}
		/*if (c == '`')
		{
			return instance->IsTidleDown;
		}*/

	}
	return false;
}

