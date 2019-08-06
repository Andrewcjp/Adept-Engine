#pragma once
#include "BaseWindow.h"
#include "Game\Gamemode.h"

class UIWidgetContext;
class UIButton;
class GameWindow : public BaseWindow
{
public:
	GameWindow();
	virtual ~GameWindow();
	void PostInitWindow(int width, int height)override;
	void InitMM();
	void SetDiffcultyPanelState(bool state);
	void SetMainPanelState(bool state);
	void DisplayDiffculty();
	void SetDifficulty(EGameDifficulty::Type type);
	void SetEasy();
	void SetHard();
	void SetMeduim();
	void ExitGame();
	void Play();
	struct PlayerSettings
	{
		std::string StartLevelName;
	};
private:
	PlayerSettings CurrentSettings;
	UIWidgetContext* MMContext = nullptr;

	UIButton* EasyButton = nullptr;
	UIButton* MeduimButton = nullptr;
	UIButton* HardButton = nullptr;
	UIButton* PlayBtn = nullptr;
	UIButton *ExitBtn = nullptr;
};

