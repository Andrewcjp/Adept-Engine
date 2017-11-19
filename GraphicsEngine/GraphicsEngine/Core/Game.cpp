#include "Game.h"

Game::Game()
{
}


Game::~Game()
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