#include "BleedOutGame.h"
#include "EngineHeader.h"
#include "Core/Components/CompoenentRegistry.h"
#include "Core/Platform/PlatformCore.h"
#include "AI/BleedOut_Director.h"
#include "BleedOutHud.h"
BleedOutGame::BleedOutGame(CompoenentRegistry* Reg) :Game(Reg)
{
	ECR = new BleedOutExtraComponentRegister();
	if (Reg != nullptr)
	{
		Reg->RegisterComponent("TGcomp", CompoenentRegistry::Limit + 1);
		Reg->RegisterComponent("PhysicsThrower", CompoenentRegistry::Limit + 2);
	}
}

BleedOutGame::~BleedOutGame()
{}

GameMode * BleedOutGame::CreateGameMode()
{
	return new BleedOutGameMode();
}

AIDirector * BleedOutGame::CreateAIDirector()
{
	return new BleedOut_Director();
}

GameHud* BleedOutGame::CreateGameHUD()
{
	return new BleedOutHud();
}
