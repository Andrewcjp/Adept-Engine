#include "TestGame.h"
#include "EngineHeader.h"
#include "../Core/Components/CompoenentRegistry.h"

TestGame::TestGame(CompoenentRegistry* Reg):Game(Reg)
{
	ECR = new TGExtraComponentRegister();
	if (Reg != nullptr)
	{
		Reg->RegisterComponent("TGcomp", CompoenentRegistry::Limit + 1);
		Reg->RegisterComponent("PhysicsThrower", CompoenentRegistry::Limit + 2);
	}
	std::cout << "hello " << std::endl;
}


TestGame::~TestGame()
{}
//use C functioning here

Game* Get(void* regvoid)
{
	return new TestGame((CompoenentRegistry*)regvoid);
}
