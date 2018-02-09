#pragma once
#include "BaseWindow.h"
class GameWindow : public BaseWindow
{
public:
	GameWindow();
	virtual ~GameWindow();
	void PostInitWindow(int width, int height)override;
};

