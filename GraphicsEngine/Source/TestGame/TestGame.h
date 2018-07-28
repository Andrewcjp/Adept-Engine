#pragma once
#include "Core\Game.h"
#include "TGExtraComponentRegister.h"
//#pragma comment(lib, "GraphicsEngine.lib")
class TestGame :
	public Game
{
public:
	TestGame(class CompoenentRegistry* Reg); 
	~TestGame();
};
   

class TestGameModule :public GameModule
{
	Game* GetGamePtr(CompoenentRegistry* Reg) override
	{
		return new TestGame(Reg);
	}
};
IMPLEMENT_MODULE(TestGameModule);