#include "stdafx.h"
#include "GameWindow.h"


GameWindow::GameWindow()
{}


GameWindow::~GameWindow()
{}

void GameWindow::PostInitWindow(int width, int height)
{
	Log::OutS  << "Game Loaded" << Log::OutS;
	IsRunning = true;
	ShouldTickScene = true;
	//LoadScene("\\asset\\scene\\testscene.scene");
	CurrentScene = new Scene();
	CurrentScene->LoadExampleScene(nullptr,false);
	Renderer->SetScene(CurrentScene);
	CurrentScene->StartScene();
}

void GameWindow::ProcessMenu(unsigned short command)
{}
