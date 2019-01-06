#include "HellKnight.h"
#include "BT_Knight.h"
#include "AI/Core/AIBase.h"
#include "AI/Core/AIController.h"
#include "HellKnight_Anim_Controller.h"

HellKnight::HellKnight()
{}

HellKnight::~HellKnight()
{}

void HellKnight::SetupBrain()
{
	BTTree = new BT_Knight();
	Controller->Speed = 15.0f;
	DeathLength = 1.2f;
	AIBase::SetupBrain();
}

AnimationController* HellKnight::CreateAnimationController()
{
	return new HellKnight_Anim_Controller();
}
