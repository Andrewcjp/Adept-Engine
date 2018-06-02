#pragma once
#include "include\glm\fwd.hpp"
#include <windows.h>
#include <queue>
#include <map>
#include "EngineGlobals.h"
class Shader_Main;
class ShaderOutput;
class OGLWindow;
class Camera;
class GameObject;
class InputEvent;
class RenderEngine;
class RenderWindow;
class EditorWindow;
class Input
{
public:
	static Input* instance;
	Input(Camera * c, GameObject * playergo, HWND window, RenderWindow* wind);
	~Input();
	void Clear();

	static void SetSelectedObject(int index);
	void ProcessQue();

	Shader_Main * main;
	ShaderOutput * Filters;


	GameObject* Selectedobject = nullptr;
	std::queue<InputEvent*> Inputque;
	int currentObjectIndex = 0;
	static void Test(Input*in);
	RenderEngine* Renderer;


	//input processing
	CORE_API static bool GetKeyDown(int c);
	CORE_API static bool GetKey(char c);
	static bool GetVKey(short key);
	static HCURSOR Cursor;
	static glm::vec2 GetMouseInputAsAxis();
	void   ProcessInput(const float delatime);
	BOOL   MouseLBDown(int x, int y);
	BOOL   MouseLBUp(int x, int y);
	BOOL   MouseMove(int x, int y, double deltatime);
	bool   ProcessKeyDown(WPARAM key);
	void   LockCursor(bool state);
private:
	bool Querry = false;
	bool FxAA = true;
	HWND m_hwnd;
	bool LockMouse = false;
	bool MSAAactive = false;
	Camera* MainCam;
	GameObject* playerGO;
	RenderWindow* ogwindow;
	EditorWindow* OpenGlwindow;	
	bool IsTidleDown = false;
	float currentmoveamt = 1.0f;//editor Movemnt
	glm::vec2 MouseAxis;
	std::map<int, bool> KeyMap;
	HKL Layout;
	bool IsActiveWindow = false;
	POINT CentrePoint;
};

class InputEvent
{
public:
	InputEvent(Input* in) { input = in; }
	~InputEvent() {}
	void(*Func)(Input*);
	Input* input;
	void FOO() {}
};

