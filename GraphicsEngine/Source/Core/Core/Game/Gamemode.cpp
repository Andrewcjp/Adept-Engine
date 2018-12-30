#include "Gamemode.h"
#include "Core/Assets/Scene.h"
#include "Core/BaseWindow.h"
#include "Core/Engine.h"
#include "Game.h"
#include "UI/GameUI/GameHud.h"

GameMode::GameMode()
{}

GameMode::~GameMode()
{}

void GameMode::BeginPlay(Scene* Scene)
{
	CurrentScene = Scene;
	Hud = Engine::GetGame()->CreateGameHUD();
	Hud->SetGameMode(this);
	Hud->Start();
}

void GameMode::EndPlay()
{
	if (Hud != nullptr)
	{
		Hud->Destory();
		SafeDelete(Hud);
	}
}

void GameMode::Update()
{
	
}

GameHud * GameMode::GetCurrentHudInstance()
{
	return Hud;
}

bool GameMode::IsGamePaused()
{
	return ((BaseWindow*)Engine::Get()->GetRenderWindow())->IsScenePaused();
}

void GameMode::OnPause()
{}

void GameMode::OnUnPause()
{}

void GameMode::AlwaysUpdate()
{
	Hud->Update();
}

void GameMode::SetPauseState(bool state)
{
	((BaseWindow*)Engine::Get()->GetRenderWindow())->SetPauseState(state);
}

void GameMode::RestartLevel()
{
	((BaseWindow*)Engine::Get()->GetRenderWindow())->EnqueueRestart();
}
