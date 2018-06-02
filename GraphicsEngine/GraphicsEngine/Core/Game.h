#pragma once
#include "EngineGlobals.h"
class  Game
{
public:
	CORE_API Game(class CompoenentRegistry* Reg);
	CORE_API ~Game();
	CORE_API virtual void  Update();
	CORE_API  void BeginPlay();
	CORE_API  void EndPlay();
	CORE_API float GetTickRate();
	CORE_API void SetTickRate(float NewRate);
	CORE_API class ExtraComponentRegister* GetECR();
protected:
	ExtraComponentRegister * ECR = nullptr;
private:
	float TickRate = 60.0f;

};

