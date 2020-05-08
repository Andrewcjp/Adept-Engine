#pragma once
#include "Core/EngineTypes.h"
#include "KeyCode.h"
#include "InputKeyboard.h"
#include "InputMouse.h"
#include "UI/Core/UIWidget.h"
namespace EInputChannel
{
	enum  Type
	{
		Game,
		UI,
		TextInput,
		Global,//for menus etc. only TextInput will reject and esc will then exit the textinput mode
		Editor,
		Limit
	};
}
class InputManager;
class TextInputHandler;
const int MAX_MOUSE_BUTTON_COUNT = 10;
class Input
{
public:
	static void Startup();
	static void ShutDown();

	CORE_API static Input* Get();
	void Clear();

	void ForceClear();

	CORE_API static glm::vec2 GetMouseInputAsAxis();
	CORE_API static IntPoint GetMousePos();
	CORE_API static bool GetKeyDown(KeyCode::Type key, EInputChannel::Type Channel = EInputChannel::Game);
	CORE_API static bool GetKey(KeyCode::Type key, EInputChannel::Type Channel = EInputChannel::Game);
	CORE_API static bool GetKeyUp(KeyCode::Type key, EInputChannel::Type Channel = EInputChannel::Game);
	CORE_API static bool GetMouseButtonDown(MouseButton::Type button, EInputChannel::Type Channel = EInputChannel::Game);
	float GetMouseWheelAxis();
	static bool GetMouseButton(MouseButton::Type button, EInputChannel::Type Channel = EInputChannel::Game);
	void ProcessInput();
	bool   MouseMove(int x, int y);
	bool   ProcessKeyDown(unsigned int key);
	bool   ProcessKeyUp(unsigned int key);
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
	static bool SendInputEvents();
	static void AddUIEvent(UIInputEvent Event);
	std::vector<UIInputEvent> Events;
	static bool CanReceiveInput(EInputChannel::Type Channel);
	static void SetInputChannel(EInputChannel::Type Channel);
	TextInputHandler* GetTextInputHandler() const { return m_pTextHandler; }
	EInputChannel::Type GetCurrentChannel() const { return m_CurrentChannel; }
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
	bool UseHighPrecisionMouseInput = true;
	int CurrentFrame = 0;
	EInputChannel::Type m_CurrentChannel = EInputChannel::Game;
	TextInputHandler* m_pTextHandler = nullptr;
};


