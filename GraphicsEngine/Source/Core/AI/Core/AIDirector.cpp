
#include "AIDirector.h"


AIDirector::AIDirector()
{

}


AIDirector::~AIDirector()
{}

void AIDirector::SetScene(Scene * newscene)
{
	scene = newscene;
}

void AIDirector::Tick()
{}

GameObject * AIDirector::GetPlayerObject()
{
	return Player.Get();
}

void AIDirector::SetPlayer(GameObject* player)
{
	Player = player;
}
