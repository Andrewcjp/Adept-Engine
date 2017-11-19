#include "stdafx.h"
#include "CompoenentRegistry.h"
#include "ExtraComponentRegister.h"
#include "Component.h"
#include "MeshRendererComponent.h"
#include "LightComponent.h"
#include "CameraComponent.h"
CompoenentRegistry* CompoenentRegistry::Instance = nullptr;
CompoenentRegistry::CompoenentRegistry()
{
	Instance = this;
}


CompoenentRegistry::~CompoenentRegistry()
{
}

Component* CompoenentRegistry::CreateAdditonalComponent(int id)
{
	if (Instance && Instance->ECR)
	{
		return Instance->ECR->CreateExtraComponent(id);
	}
	return nullptr;
}
Component* CompoenentRegistry::CreateBaseComponent(BaseComponentTypes id)
{
	if (Instance)
	{
		return Instance->Internal_CreateBaseComponent(id);
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
	}
	return nullptr;
}
Component* CompoenentRegistry::Internal_CreateAdditonalComponent(int id)
{
	return ECR->CreateExtraComponent(id);
}

void CompoenentRegistry::RegisterExtraComponents(ExtraComponentRegister * Ecr)
{
	if (ECR)
	{
		ECR = Ecr;
		AdditonalGameComponents = ECR->GetExtraCompIds();
	}
}
