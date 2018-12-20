#pragma once
#include "Core/Components/Component.h"
#include "Core/Types/WeakObjectPtr.h"
class AIController;
class BehaviourTree;
class AnimationController;
class AIBase : public Component
{
public:
	CORE_API AIBase();
	CORE_API virtual ~AIBase();
	//todo: AI director will know this/search scene for tagged object
	WeakObjectPtr<GameObject> Player;

	CORE_API virtual void OnDestroy() override;
	AnimationController* GetAnimController()const
	{
		return AnimController;
	}
	///Called when health depleted - disables this AI and plays the Death anim
	CORE_API void SetDead();
protected:
	CORE_API virtual void SetBrainEnabled(bool state);
	CORE_API virtual void SetupBrain();
	CORE_API virtual void Update(float dt) override;
	CORE_API virtual void InitComponent() override;
	float DistanceToPlayer = -1.0f;
	AIController* Controller = nullptr;
	BehaviourTree* BTTree = nullptr;
	AnimationController* AnimController = nullptr;
	CORE_API virtual AnimationController* CreateAnimationController();
	float DeathLength = 1.0f;
private:
	bool Active = true;
	float DeathTimer = 0.0f;
	bool IsDying = false;
};

