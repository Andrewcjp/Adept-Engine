#include "stdafx.h"
#include "Component.h"
#include "Core/GameObject.h"
#include "Core/Assets/Archive.h"
#include "Core/Platform/PlatformCore.h"
Component::Component()
{
	TypeID = -1;
}


Component::~Component()
{
}

void Component::InitComponent()
{
}

void Component::BeginPlay()
{}

void Component::Update(float delta)
{}

void Component::FixedUpdate(float )
{
}
#if WITH_EDITOR
void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Component");
	//group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Float, nullptr));
	props.push_back(group);
}
#endif
GameObject * Component::GetOwner()
{
	return Owner;
}

void Component::Internal_SetOwner(GameObject * ptr)
{
	ensureMsgf((Owner == nullptr), "This component Already Has an owner");
	Owner = ptr;
	InitComponent();
}

Scene * Component::GetOwnerScene()
{
	if (GetOwner() != nullptr)
	{
		return GetOwner()->GetScene();
	}
	return nullptr;
}

void Component::OnTransformUpdate()
{
}

void Component::ProcessSerialArchive(Archive * A)
{
	if (!A->IsReading())
	{
		ArchiveProp(TypeID);
	}
}

