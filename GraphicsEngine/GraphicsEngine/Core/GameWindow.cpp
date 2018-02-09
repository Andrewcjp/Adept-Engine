#include "stdafx.h"
#include "GameWindow.h"


GameWindow::GameWindow()
{}


GameWindow::~GameWindow()
{}

void GameWindow::PostInitWindow(int width, int height)
{
	std::cout << "Game Loaded" << std::endl;
	IsRunning = true;
	ShouldTickScene = true;
}
