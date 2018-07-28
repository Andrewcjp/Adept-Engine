#include "stdafx.h"
#include "Component.h"
#include "Core/GameObject.h"
#include "Core/Assets/SerialHelpers.h"
#include "Core/Assets/SceneJSerialiser.h"

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

void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Component");
	//group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Float, nullptr));
	props.push_back(group);
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

