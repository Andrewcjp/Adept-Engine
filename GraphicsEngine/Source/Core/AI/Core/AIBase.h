#pragma once
#include "Core/Components/Component.h"
class AIBase : public Component
{
public:
	CORE_API AIBase();
	CORE_API ~AIBase();
	//todo: AI director will know this/search scene for tagged object
	GameObject* Player = nullptr;
protected:
	CORE_API virtual void Update(float dt) override;
	float DistanceToPlayer = -1.0f;
	
private:
	CORE_API virtual void InitComponent() override;
	
};

