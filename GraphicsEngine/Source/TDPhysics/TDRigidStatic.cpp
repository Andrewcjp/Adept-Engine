#include "TDRigidStatic.h"


namespace TD
{
	TDRigidStatic::TDRigidStatic()
	{
		ActorType = TDActorType::RigidStatic;
		BodyMass = 0.0f;//zero mass forces no responses in collision engine 
	}


	TDRigidStatic::~TDRigidStatic()
	{}
}