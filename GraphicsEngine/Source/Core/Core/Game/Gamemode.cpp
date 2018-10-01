#include "Stdafx.h"
#include "Gamemode.h"


GameMode::GameMode()
{}


GameMode::~GameMode()
{}

void GameMode::BeginPlay(Scene* Scene)
{
	CurrentScene = Scene;
}

void GameMode::EndPlay()
{}

void GameMode::Update()
{}
