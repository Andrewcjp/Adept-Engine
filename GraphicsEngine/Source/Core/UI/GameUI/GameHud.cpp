#include "Source/Core/Stdafx.h"
#include "GameHud.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/UIManager.h"


GameHud::GameHud()
{}


GameHud::~GameHud()
{}

void GameHud::Start()
{
	Context = new UIWidgetContext();
	UIManager::Get()->AddWidgetContext(Context);
	OnStart();
}

void GameHud::Update()
{
	OnUpdate();
}

void GameHud::Destory()
{
	UIManager::Get()->RemoveWidgetContext(Context);
	SafeDelete(Context);
	OnDestory();
}

void GameHud::SetGameMode(GameMode * gm)
{
	gameMode = gm;
}

void GameHud::OnDestory()
{}

void GameHud::OnStart()
{}

void GameHud::OnUpdate()
{}
