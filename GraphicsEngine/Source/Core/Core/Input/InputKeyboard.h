#pragma once
namespace KeyCode
{
	enum Type
	{
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		RightSquareBracket,
		LeftSquareBracket,
		Hash,
		Backslash,
		Tidle,//`
		Escape,
		Control,
		Shift,
		Limit
	};
}
class InputKeyboard
{
public:
	InputKeyboard();
	virtual ~InputKeyboard();
	void Update();
	bool IsKeyDown(KeyCode::Type code);
	bool IsKey(KeyCode::Type code);
	bool IsKeyUp(KeyCode::Type code);
protected:
	virtual void UpdateState() {};
	bool KeyBoardstate[KeyCode::Limit] = {};
	bool LastKeyBoardState[KeyCode::Limit] = {};
};

