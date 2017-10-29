#pragma once
#include <vector>
#include "../Editor/Inspector.h"
class Component
{
public:
	Component();
	virtual ~Component();
	virtual void BeginPlay() = 0;
	virtual void Update(float delta) = 0;
	virtual void FixedUpdate(float delta);
	virtual void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props);
	bool DoesUpdate = true;
	bool DoesFixedUpdate = false;
};

