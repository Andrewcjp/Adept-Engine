#include "stdafx.h"
#include "Component.h"
#include "Core/GameObject.h"
#include "Core/Assets/Archive.h"
#include "Core/Platform/PlatformCore.h"
Component::Component()
{}

Component::~Component()
{}

void Component::BeginPlay()
{}

void Component::Update(float delta)
{}

void Component::FixedUpdate(float)
{}

#if WITH_EDITOR
void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props)
{}
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
{}

void Component::ProcessSerialArchive(Archive * A)
{
	if (!A->IsReading())
	{
		ArchiveProp(TypeID);
	}
}

void Component::OnDestroy()
{}