#pragma once
#include "AI/Core/Behaviour/BehaviourTree.h"
class BT_PossessedSoldier : public BehaviourTree
{
public:
	BT_PossessedSoldier();
	~BT_PossessedSoldier();

	virtual void SetupTree() override;
};

