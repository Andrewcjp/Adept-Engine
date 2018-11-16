#pragma once
#include "Core/Components/Component.h"
class AIController;
class BehaviourTree;
class AIBase : public Component
{
public:
	CORE_API AIBase();
	CORE_API ~AIBase();
	//todo: AI director will know this/search scene for tagged object
	GameObject* Player = nullptr;

	CORE_API virtual void OnDestroy() override;

protected:
	CORE_API virtual void SetupBrain();
	CORE_API virtual void Update(float dt) override;
	float DistanceToPlayer = -1.0f;
	AIController* Controller = nullptr;
	BehaviourTree* BTTree = nullptr;
private:
	CORE_API virtual void InitComponent() override;

	
};

