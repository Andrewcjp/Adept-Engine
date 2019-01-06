#pragma once
#include "AI/Core/Behaviour/BehaviourTree.h"
/*!  \addtogroup Game_AI
* @{ */
class BT_Knight :public BehaviourTree
{
public:
	BT_Knight();
	~BT_Knight();

	virtual void SetupTree() override;

};

