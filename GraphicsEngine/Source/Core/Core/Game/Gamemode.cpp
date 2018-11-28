#include "Gamemode.h"
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
	Hud->Update();
}

GameHud * GameMode::GetCurrentHudInstance()
{
	return Hud;
}
