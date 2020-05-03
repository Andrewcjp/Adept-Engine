#include "InputKeyboard.h"


InputKeyboard::InputKeyboard()
{
}


InputKeyboard::~InputKeyboard()
{
}

void InputKeyboard::Update()
{
	memcpy(&LastKeyBoardState, &KeyBoardstate, KeyCode::Limit);
	memset(&KeyBoardstate, 0, KeyCode::Limit);
	UpdateState();
}

bool InputKeyboard::IsKeyDown(KeyCode::Type code)
{
	return KeyBoardstate[code] && !LastKeyBoardState[code]; 
}

bool InputKeyboard::IsKey(KeyCode::Type code)
{
	return KeyBoardstate[code];
}

bool InputKeyboard::IsKeyUp(KeyCode::Type code)
{
	return !KeyBoardstate[code] && LastKeyBoardState[code];
}
