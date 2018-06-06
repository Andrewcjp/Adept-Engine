#pragma once
#include "Core\Game.h"
#include "TGExtraComponentRegister.h"
#pragma comment(lib, "GraphicsEngine.lib")
class TestGame :
	public Game
{
public:
	TestGame(class CompoenentRegistry* Reg); 
	~TestGame();
};
   
extern "C" __declspec(dllexport) Game* Get(void* regvoid);    