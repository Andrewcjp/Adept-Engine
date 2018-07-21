#pragma once
#include "include\glm\fwd.hpp"

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
const int MAX_MOUSE_BUTTON_COUNT = 10;
class Input
{
public:
	static Input* instance;
	Input();
	~Input();

	void Clear();
	void ProcessQue();
	std::queue<InputEvent*> Inputque;

	//input processing
	CORE_API static bool GetKeyDown(int c);
	CORE_API static bool GetKey(char c);
	static bool GetVKey(short key);
	
	static glm::vec2 GetMouseInputAsAxis();
	void   ProcessInput(const float delatime);
	bool   MouseLBDown(int x, int y);
	bool   MouseLBUp(int x, int y);
	bool   MouseMove(int x, int y, double deltatime);
	bool   ProcessKeyDown(unsigned int key);
	void   LockCursor(bool state);
	static void ReciveMouseDownMessage(int Button, bool state);
	static bool GetMouseButtonDown(int button);
private:
	bool LockMouse = false;
	float currentmoveamt = 1.0f;//editor Movemnt
	glm::vec2 MouseAxis;
	std::map<int, bool> KeyMap;	
	bool IsActiveWindow = false;
	IntPoint CentrePoint;
	bool MouseKeyData[MAX_MOUSE_BUTTON_COUNT] = { false };
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

