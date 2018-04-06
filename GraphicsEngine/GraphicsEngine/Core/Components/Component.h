#pragma once
#include <vector>
#include "../Editor/Inspector.h"
#include "rapidjson\document.h"
class Component
{
public:

	Component();
	virtual ~Component();
	virtual void InitComponent() = 0;
	virtual void BeginPlay() = 0;
	virtual void Update(float delta) = 0;
	virtual void FixedUpdate(float delta);
	virtual void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props);
	class GameObject* GetOwner();
	void Internal_SetOwner(GameObject* ptr);
	bool DoesUpdate = true;
	bool DoesFixedUpdate = false;
	virtual void OnTransformUpdate();
	virtual void Serialise(rapidjson::Value& v);
	virtual void Deserialise(rapidjson::Value& v) = 0;
	virtual void SceneInitComponent() {};
	virtual void PostChangeProperties() {};
protected:
	int TypeID = -1;
private:
	GameObject* Owner = nullptr;
	
};

