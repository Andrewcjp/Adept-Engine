#include "Game.h"

Game::Game(CompoenentRegistry* Reg)
{
}


Game::~Game()
{
}
void Game::Init()
{
	
}
void Game::Update()
{
	
}

void Game::BeginPlay()
{
	
}

void Game::EndPlay()
{
	
}

float Game::GetTickRate()
{
	return TickRate;
}

void Game::SetTickRate(float NewRate)
{
	TickRate = NewRate;
}
ExtraComponentRegister * Game::GetECR()
{
	return ECR;
}

GameMode * Game::CreateGameMode()
{
	return new GameMode();
}
