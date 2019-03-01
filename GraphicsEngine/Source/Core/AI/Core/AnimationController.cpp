#include "AnimationController.h"
#include "Core/Assets/MeshLoader.h"
#include "Core/Components/Component.h"
#include "Core/GameObject.h"
#include "Core/Platform/Logger.h"
#include "Physics/SimTD/TDRigidBody.h"
#ifndef WITH_UNITY
#include "AI/Core/AIBase.h"
#endif


AnimationController::AnimationController()
{
	InitDefaultStateMap();
}

AnimationController::~AnimationController()
{}

void AnimationController::InitDefaultStateMap()
{
	StateMap.emplace((int)EGenericAnimtionStates::Idle, new AnimationState("Idle"));
	StateMap.emplace((int)EGenericAnimtionStates::Attack, new AnimationState("Attack"));
	StateMap.emplace((int)EGenericAnimtionStates::Walking, new AnimationState("Walking"));
	StateMap.emplace((int)EGenericAnimtionStates::Dead, new AnimationState("Death"));
}

void AnimationController::SetState(EGenericAnimtionStates::Type state)
{
	if (GetCurrentEnumState() == state)
	{
		return;
	}
	//one of the few okay uses of auto 
	auto Itor = StateMap.find((int)state);
	if (Itor != StateMap.end())
	{
		TransitionToState(Itor->second);
		CurrentEnumState = state;
	}
	else
	{
		Log::LogMessage("Failed to Transition to state");
	}
}

void AnimationController::OnTick(float dt)
{
	if (Owner != nullptr)
	{
		if (CurrentEnumState == EGenericAnimtionStates::Dead)//One way 
		{
			return;
		}
		if (AttackTimer > 0.0f)
		{
			AttackTimer -= dt;
			SetState(EGenericAnimtionStates::Attack);
			return;
		}
		glm::vec3 vel = Owner->GetOwner()->GetRigidbody()->GetLinearVelocity();
		vel.y = 0.0f;
		float Velcity = glm::length2(vel);
		if (Velcity >= WalkSpeed * WalkSpeed)
		{
			SetState(EGenericAnimtionStates::Walking);
		}
		else
		{
			SetState(EGenericAnimtionStates::Idle);
		}
	}
}

void AnimationController::TransitionToState(AnimationState * NewState)
{
	CurrnetState = NewState;
	Mesh->PlayAnimation(CurrnetState->AnimName);
	//todo: transitions
}

void AnimationController::Tick(float Time)
{
	//if (CurrnetState != nullptr)
	//{
	//	CurrnetState->Tick();
	//}
	OnTick(Time);
}

void AnimationController::TriggerAttack(float length)
{
	AttackTimer = length;
}

void Transisiton::SetDefault()
{}

void Transisiton::Tick(float Dt)
{

}

void Transisiton::Reset()
{
	CurrentTime = 0.0f;
	Length = 0.0f;
}

void Transisiton::SetUp(AnimationState * src, AnimationState * dst)
{
	Reset();
	Src = src;
	Dst = dst;
}

bool Transisiton::IsComplete() const
{
	return Complete;
}
