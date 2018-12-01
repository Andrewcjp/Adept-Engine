#include "HellKnight.h"
#include "BT_Knight.h"
#include "AI/Core/AIBase.h"

HellKnight::HellKnight()
{}


HellKnight::~HellKnight()
{}

void HellKnight::SetupBrain()
{
	BTTree = new BT_Knight();
	AIBase::SetupBrain();
}
