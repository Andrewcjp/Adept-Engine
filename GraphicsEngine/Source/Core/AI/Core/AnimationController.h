#pragma once
/*! \defgroup AI Engine AI Classes
*\addtogroup AI
* @{ */
struct SkeletalMeshEntry;
class AIBase;
///This class Acts as a base for animation controllers
class AnimationState
{
public:
	AnimationState()
	{}
	AnimationState(std::string name)
	{
		AnimName = name;
	}
	std::string AnimName = "";
	bool Loop = false;
	float TransitionTime = 0.0f;
};
class Transisiton
{
	void SetDefault();
	void Tick(float Dt);
	void Reset();
	void SetUp(AnimationState* Src, AnimationState* Dst);
	bool IsComplete()const;
private:
	AnimationState* Src = nullptr;
	AnimationState* Dst = nullptr;
	float Length = 0.0f;
	float CurrentTime = 0.0f;
	bool Complete;
};
namespace EGenericAnimtionStates
{
	enum Type
	{
		Idle,
		Walking,
		Attack,
		Dead,
		Limit
	};
}
class AnimationController
{
public:
	CORE_API AnimationController();
	CORE_API virtual ~AnimationController();
	void InitDefaultStateMap();
	void SetState(EGenericAnimtionStates::Type state);
	CORE_API virtual void OnTick(float dt);
	void Tick(float Time);
	SkeletalMeshEntry* Mesh = nullptr;
	AIBase* Owner = nullptr;
	float WalkSpeed = 5.0f;
	float AttackTimer = 0.0f;
	CORE_API void TriggerAttack(float length);
	EGenericAnimtionStates::Type GetCurrentEnumState()const
	{
		return CurrentEnumState;
	}
protected:
	void TransitionToState(AnimationState* NewState);
private:
	AnimationState* CurrnetState = nullptr;
	Transisiton CurrentTransition;
	std::map<int, AnimationState*> StateMap;
	EGenericAnimtionStates::Type CurrentEnumState = EGenericAnimtionStates::Limit;
};

