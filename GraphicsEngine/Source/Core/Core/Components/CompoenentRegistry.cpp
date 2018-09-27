#include "stdafx.h"
#include "CompoenentRegistry.h"
#include "ExtraComponentRegister.h"
#include "Component.h"
#include "MeshRendererComponent.h"
#include "LightComponent.h"
#include "CameraComponent.h"
#include "Core/Engine.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
CompoenentRegistry::CompoenentRegistry()
{
	Engine::CompRegistry = this;
	RegisterComponent("Camera", BaseComponentTypes::CameraComp);
	RegisterComponent("Light", BaseComponentTypes::LightComp);
	RegisterComponent("RigidBody", BaseComponentTypes::RigidComp);
	RegisterComponent("Particle System", BaseComponentTypes::ParticleComp);
}

CompoenentRegistry::~CompoenentRegistry()
{
}
void CompoenentRegistry::RegisterComponent(std::string name, int id)
{
	ComponentNameMap.emplace(id, name);
}
CompoenentRegistry * CompoenentRegistry::GetInstance()
{
	return Engine::CompRegistry;
}
Component* CompoenentRegistry::CreateAdditonalComponent(int id)
{
	if (Engine::CompRegistry && Engine::CompRegistry->ECR)
	{
		return Engine::CompRegistry->ECR->CreateExtraComponent(id);
	}
	return nullptr;
}
Component* CompoenentRegistry::CreateBaseComponent(BaseComponentTypes id)
{
	if (Engine::CompRegistry)
	{
		if (id < BaseComponentTypes::Limit)
		{
			return Engine::CompRegistry->Internal_CreateBaseComponent(id);
		}
		return Engine::CompRegistry->Internal_CreateAdditonalComponent(id - (BaseComponentTypes::Limit + 1));//translate the ID to GameLocal space
	}
	return nullptr;
}
Component* CompoenentRegistry::Internal_CreateBaseComponent(BaseComponentTypes id)
{
	switch (id)
	{
	case MeshComp:
		return new MeshRendererComponent();
		break;
	case LightComp:
		return new LightComponent();
		break;
	case CameraComp:
		return new CameraComponent();
		break;
	case RigidComp:
		return new RigidbodyComponent();
		break;
	case ColliderComp:
		return new ColliderComponent();
		break;
	}
	return nullptr;
}
Component* CompoenentRegistry::Internal_CreateAdditonalComponent(int id)
{
	return ECR->CreateExtraComponent(id);
}

void CompoenentRegistry::RegisterExtraComponents(ExtraComponentRegister * Ecr)
{
	if (Ecr != nullptr)
	{
		ECR = Ecr;
		AdditonalGameComponents = ECR->GetExtraCompIds();
	}
}

std::string CompoenentRegistry::GetNameById(int id)
{
	if (ComponentNameMap.find(id) != ComponentNameMap.end())
	{
		return ComponentNameMap.at(id);
	}
	return "UnNamed";
}

int CompoenentRegistry::GetCount()
{
	return BaseComponentTypes::Limit + 3;
}
