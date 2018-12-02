#include "HellKnight.h"
#include "BT_Knight.h"
#include "AI/Core/AIBase.h"
#include "AI/Core/AIController.h"

HellKnight::HellKnight()
{}

HellKnight::~HellKnight()
{}

void HellKnight::SetupBrain()
{
	BTTree = new BT_Knight();
	Controller->Speed = 15.0f;
	AIBase::SetupBrain();
}
