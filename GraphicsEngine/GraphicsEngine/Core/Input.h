#pragma once
#include "include\glm\fwd.hpp"
#include <Windows.h>
#include <queue>
#include <map>
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
	void CreateStackAtPoint();

	void FireAtScene();
	void Clear();

	static void SetSelectedObject(int index);
	void ProcessQue();

	Shader_Main * main;
	ShaderOutput * Filters;
	float GetForce() { return CurrentForce / 10000; }
	int GetShape() { return currentshape; }
	GameObject* Selectedobject = nullptr;
	std::queue<InputEvent*> Inputque;
	int currentObjectIndex = 0;
	static void Test(Input*in);
	RenderEngine* Renderer;


	//input processing
	static bool GetKeyDown(int c);
	static bool GetKey(char c);
	static bool GetVKey(short key);
	static HCURSOR Cursor;
	static glm::vec2 GetMouseInputAsAxis();
	void   ProcessInput(const float delatime);
	BOOL   MouseLBDown(int x, int y);
	BOOL   MouseLBUp(int x, int y);
	BOOL   MouseMove(int x, int y, double deltatime);
	BOOL   ProcessKeyDown(WPARAM key);
private:
	bool Querry = false;
	bool FxAA = true;
	HWND m_hwnd;
	bool FreeCamMode = true;
	bool MouseLookActive = false;
	bool MSAAactive = false;
	Camera* MainCam;
	GameObject* playerGO;
	RenderWindow* ogwindow;
	EditorWindow* OpenGlwindow;
	float CurrentForce = 10000;
	int currentshape = 0;
	bool IsTidleDown = false;
	float currentmoveamt = 1;
	glm::vec2 MouseAxis;
	std::map<int, bool> KeyMap;
	HKL Layout;
	bool IsActiveWindow = false;
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

