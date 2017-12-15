#include "stdafx.h"
#include "Component.h"
#include "../GameObject.h"
#include "../Assets/SerialHelpers.h"
#include "../Assets/SceneJSerialiser.h"
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

void Component::FixedUpdate(float )
{
}

void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &)
{
}

GameObject * Component::GetOwner()
{
	return Owner;
}

void Component::Internal_SetOwner(GameObject * ptr)
{
	assert((Owner == nullptr) && "This component Already Has an owner");
	Owner = ptr;
	InitComponent();
}

void Component::OnTransformUpdate()
{
}

void Component::Serialise(rapidjson::Value & v)
{
	SerialHelpers::addLiteral(v, *SceneJSerialiser::jallocator, "Type", TypeID);
}

