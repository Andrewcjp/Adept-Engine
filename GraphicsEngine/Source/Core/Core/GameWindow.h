#pragma once
#include "BaseWindow.h"

class UIWidgetContext;
class GameWindow : public BaseWindow
{
public:
	GameWindow();
	virtual ~GameWindow();
	void PostInitWindow(int width, int height)override;
	void InitMM();
	void ExitGame();
	void Play();
	struct PlayerSettings
	{
		std::string StartLevelName;
	};
private:
	PlayerSettings CurrentSettings;
	UIWidgetContext* MMContext = nullptr;
	// Inherited via BaseWindow
	virtual void ProcessMenu(unsigned short command) override;
};

