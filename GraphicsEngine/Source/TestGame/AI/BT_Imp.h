#pragma once
#include "AI/Core/Behaviour/BehaviourTree.h"
class BT_Imp :public BehaviourTree
{
public:
	BT_Imp();
	~BT_Imp();

	virtual void SetupTree() override;

};

