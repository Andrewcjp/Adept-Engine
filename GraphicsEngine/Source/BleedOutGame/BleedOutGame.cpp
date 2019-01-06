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

std::vector<std::string> BleedOutGame::GetPreLoadAssets()
{
	std::vector<std::string> vector;
	vector.push_back("Texture\\vanguard_diffuse.png");
	vector.push_back("Texture\\Mutant_diffuse.png");
	vector.push_back("\\texture\\ammoc03.jpg");
	vector.push_back("Texture\\ProjectileTex.png");
	return vector;
}
