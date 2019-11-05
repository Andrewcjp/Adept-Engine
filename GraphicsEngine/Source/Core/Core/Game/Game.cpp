#include "Game.h"
#include "ai/CORE/AIDirector.h"
#include "UI/GameUI/GameHud.h"
Game::Game(ComponentRegistry* Reg)
{}


Game::~Game()
{}
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

AIDirector * Game::CreateAIDirector()
{
	return new AIDirector();
}

GameHud * Game::CreateGameHUD()
{
	return new GameHud();
}

std::vector<std::string> Game::GetPreLoadAssets()
{
	std::vector<std::string> debug;
	debug.push_back("Props\\Crate_2\\Crate_Diffuse.png");
	debug.push_back("\\texture\\MarsSky.dds");
	debug.push_back("Props\\Crate_1\\low_default_AlbedoTransparency.png");
	debug.push_back("\\texture\\BoxObject.png");
	debug.push_back("\\Terrain\\textures_industrial_floors_floor_paint_lightgray_c.png");
	debug.push_back("\\texture\\bricks2.jpg");
	debug.push_back("\\Terrain\\DoorTex.png");
	return debug;
}
