#pragma once
#include <vector>
#include "Editor/Inspector.h"
#include "rapidjson\document.h"
#include "EngineGlobals.h"

class Component
{
public:

	CORE_API Component();
	CORE_API virtual ~Component();
	CORE_API virtual void InitComponent() = 0;
	CORE_API virtual void BeginPlay() = 0;
	CORE_API virtual void Update(float delta) = 0;
	CORE_API virtual void FixedUpdate(float delta);
	CORE_API virtual void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props);
	CORE_API class GameObject* GetOwner();
	void Internal_SetOwner(GameObject* ptr);
	bool DoesUpdate = true;
	bool DoesFixedUpdate = false;
	CORE_API virtual void OnTransformUpdate();
	CORE_API virtual void Serialise(rapidjson::Value& v);
	CORE_API virtual void Deserialise(rapidjson::Value& v) = 0;
	CORE_API virtual void SceneInitComponent() {};
	CORE_API virtual void PostChangeProperties() {};
protected:
	int TypeID = -1;
private:
	GameObject* Owner = nullptr;
	
};

