#pragma once
#include "AI/Core/Behaviour/BehaviourTree.h"
/*!  \addtogroup Game_AI
* @{ */
class BT_PossessedSoldier : public BehaviourTree
{
public:
	BT_PossessedSoldier();
	~BT_PossessedSoldier();

	virtual void SetupTree() override;
};

