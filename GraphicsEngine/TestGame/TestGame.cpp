#include "TestGame.h"
#include "EngineHeader.h"
#include "../Core/Components/CompoenentRegistry.h"

TestGame::TestGame()
{
	ECR = new TGExtraComponentRegister();
	if (CompoenentRegistry::Instance != nullptr)
	{
		CompoenentRegistry::Instance->RegisterComponent("TGcomp", CompoenentRegistry::Limit + 1);
	}
}


TestGame::~TestGame()
{}
