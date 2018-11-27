#include "TestGame.h"
#include "EngineHeader.h"
#include "Core/Components/CompoenentRegistry.h"
#include "Core/Platform/PlatformCore.h"
#include "AI/TestGame_Director.h"
#include "TestGameHud.h"
TestGame::TestGame(CompoenentRegistry* Reg) :Game(Reg)
{
	ECR = new TGExtraComponentRegister();
	if (Reg != nullptr)
	{
		Reg->RegisterComponent("TGcomp", CompoenentRegistry::Limit + 1);
		Reg->RegisterComponent("PhysicsThrower", CompoenentRegistry::Limit + 2);
	}
}

TestGame::~TestGame()
{}

GameMode * TestGame::CreateGameMode()
{
	return new TestGameGameMode();
}

AIDirector * TestGame::CreateAIDirector()
{
	return new TestGame_Director();
}

GameHud* TestGame::CreateGameHUD()
{
	return new TestGameHud();
}
