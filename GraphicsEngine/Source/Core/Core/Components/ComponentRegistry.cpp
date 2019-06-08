#include "ComponentRegistry.h"
#include "ExtraComponentRegister.h"
#include "Component.h"
#include "MeshRendererComponent.h"
#include "LightComponent.h"
#include "CameraComponent.h"
#include "Core/Engine.h"
#include "RigidbodyComponent.h"
#include "ColliderComponent.h"
#include "AI/Core/SpawnMarker.h"
ComponentRegistry::ComponentRegistry()
{
	Engine::CompRegistry = this;
	RegisterComponent("Camera", BaseComponentTypes::CameraComp);
	RegisterComponent("Light", BaseComponentTypes::LightComp);
	RegisterComponent("RigidBody", BaseComponentTypes::RigidComp);
	RegisterComponent("Particle System", BaseComponentTypes::ParticleComp);
}

ComponentRegistry::~ComponentRegistry()
{}
void ComponentRegistry::RegisterComponent(std::string name, int id)
{
	ComponentNameMap.emplace(id, name);
}
ComponentRegistry * ComponentRegistry::GetInstance()
{
	return Engine::CompRegistry;
}
Component* ComponentRegistry::CreateAdditonalComponent(int id)
{
	if (Engine::CompRegistry && Engine::CompRegistry->ECR)
	{
		return Engine::CompRegistry->ECR->CreateExtraComponent(id);
	}
	return nullptr;
}
Component* ComponentRegistry::CreateBaseComponent(BaseComponentTypes id)
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
Component* ComponentRegistry::Internal_CreateBaseComponent(BaseComponentTypes id)
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
	case SpawnMarkerComp:
		return new SpawnMarker();
	}
	return nullptr;
}
Component* ComponentRegistry::Internal_CreateAdditonalComponent(int id)
{
	return ECR->CreateExtraComponent(id);
}

void ComponentRegistry::RegisterExtraComponents(ExtraComponentRegister * Ecr)
{
	if (Ecr != nullptr)
	{
		ECR = Ecr;
		AdditonalGameComponents = ECR->GetExtraCompIds();
	}
}

std::string ComponentRegistry::GetNameById(int id)
{
	if (ComponentNameMap.find(id) != ComponentNameMap.end())
	{
		return ComponentNameMap.at(id);
	}
	return "UnNamed";
}

int ComponentRegistry::GetCount()
{
	return BaseComponentTypes::Limit + 3;
}
