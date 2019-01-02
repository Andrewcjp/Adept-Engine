#include "AttackController.h"
#include "TestGame_Director.h"

AttackController::AttackController()
{}


AttackController::~AttackController()
{}

bool AttackController::CanGetToken(AIBase * Base)
{
	if (VectorUtils::Contains(CurrentAITokens, Base))
	{
		return true;
	}
	if (CurrentAITokens.size() + 1 <= OwningDirector->GetMaxAttackingAI())
	{
		CurrentAITokens.push_back(Base);
		return true;
	}
	return false;
}

void AttackController::ReleaseToken(AIBase * Base)
{
	VectorUtils::Remove(CurrentAITokens, Base);
}
