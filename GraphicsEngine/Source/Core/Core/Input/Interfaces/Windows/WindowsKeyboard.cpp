#include "WindowsKeyboard.h"
#include "Core/Input/KeyCode.h"


WindowsKeyboard::WindowsKeyboard()
{
}


WindowsKeyboard::~WindowsKeyboard()
{
}
int KeyMappingTable[]
{
		 65,
		66,
		67,
		68,
		69,
		70,
		 71,
		 72,
		 73,
		 74,
		 75,
		 76,
		 77,
		 78,
		 79,
		 80,
		 81,
		 82,
		 83,
		 84,
		 85,
		 86,
		 87,
		 88,
		 89,
		 90,
		 VK_F1,
		 VK_F2,
		 VK_F3,
		 VK_F4,
		 VK_F5,
		 VK_F6,
		 VK_F7,
		 VK_F8,
		 VK_F9,
		 VK_F10,
		 VK_F11,
		 VK_F12,
		 ']',
		 '[',
		 '#',
		 '\'',
		 VK_OEM_8,
		 VK_ESCAPE,
		 VK_CONTROL,
		 VK_SHIFT,
		 VK_UP,
		 VK_DOWN,
};

void WindowsKeyboard::UpdateState()
{
	if (!WindowsWindow::IsActiveWindow())
	{
		return;
	}
	for (int i = 0; i < KeyCode::Limit; i++)
	{
		KeyBoardstate[i] = (GetKeyState(KeyMappingTable[i]) & 0x8000);
	}
}
