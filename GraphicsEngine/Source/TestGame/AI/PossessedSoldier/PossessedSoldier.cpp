#include "Source/TestGame/TestGamePCH.h"
#include "PossessedSoldier.h"
#include "BT_PossessedSoldier.h"
#include "AI/Core/AIController.h"

PossessedSoldier::PossessedSoldier()
{}


PossessedSoldier::~PossessedSoldier()
{}
void PossessedSoldier::SetupBrain()
{
	BTTree = new BT_PossessedSoldier();
	Controller->Speed = 10.0f;
	AIBase::SetupBrain();
}
