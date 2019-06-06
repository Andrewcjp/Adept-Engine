#pragma once
#include "Core/EngineTypes.h"
#include "KeyCode.h"

class InputManager;
const int MAX_MOUSE_BUTTON_COUNT = 10;
class Input
{
public:
	static void Startup();
	static void ShutDown();

	CORE_API static Input* Get();
	void Clear();

	void ForceClear();

	//input processing
	CORE_API static bool GetKeyDown(int c);
	CORE_API static bool GetKeyUp(int c);
	CORE_API static bool GetKey(char c);
	CORE_API static bool GetVKey(short key);
	CORE_API static glm::vec2 GetMouseInputAsAxis();
	CORE_API static IntPoint GetMousePos();

	void ProcessInput();
	bool   MouseLBDown(int x, int y);
	bool   MouseLBUp(int x, int y);
	bool   MouseMove(int x, int y, double deltatime);
	bool   ProcessKeyDown(unsigned int key);
	bool   ProcessKeyUp(unsigned int key);
	void   ProcessMouseWheel(float Delta);
	CORE_API static void ReciveMouseDownMessage(int Button, bool state);
	CORE_API static bool GetMouseButtonDown(int button);
	CORE_API static void SetCursorVisible(bool state);
	bool IsUsingHPMI()
	{
		return UseHighPrecisionMouseInput;
	};
	void ReciveMouseAxisData(glm::vec2 data);
	CORE_API static void SetCursorState(bool Locked, bool Visible);
	static void LockCursor(bool state);
	CORE_API static bool GetMouseWheelUp();
	CORE_API static bool GetMouseWheelDown();

	void ResetMouse();
	int MouseSampleCount = 0;
	bool DidJustPause = false;
	static InputManager* GetInputManager();
private:
	InputManager* IManager = nullptr;
	static Input* instance;
	Input();
	~Input();
	bool LockMouse = false;
	glm::vec2 MouseAxis;
	IntPoint MousePosScreen;
	std::map<int, bool> KeyMap;
	bool IsActiveWindow = true;
	IntPoint CentrePoint;
	bool MouseKeyData[MAX_MOUSE_BUTTON_COUNT] = { false };
	bool UseHighPrecisionMouseInput = true;
	int CurrentFrame = 0;
	bool MouseWheelUpThisFrame = false;
	bool MouseWheelDownThisFrame = false;
};


