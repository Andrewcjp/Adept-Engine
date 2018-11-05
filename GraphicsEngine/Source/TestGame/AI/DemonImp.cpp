#include "DemonImp.h"
#include "BT_Imp.h"
#include "AI/Core/AIBase.h"

DemonImp::DemonImp()
{}


DemonImp::~DemonImp()
{}

void DemonImp::SetupBrain()
{
	BTTree = new BT_Imp();
	AIBase::SetupBrain();
}
