#pragma once
#include <vector>
#include "Editor/Inspector.h"
class Component
{
public:
	CORE_API Component();
	CORE_API virtual ~Component();
	CORE_API virtual void InitComponent() = 0;
	CORE_API virtual void BeginPlay();
	CORE_API virtual void Update(float delta);
	CORE_API virtual void FixedUpdate(float delta);
	CORE_API virtual void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props);
	CORE_API class GameObject* GetOwner();

	CORE_API virtual void OnTransformUpdate();
	CORE_API virtual void SceneInitComponent() {};
	CORE_API virtual void PostChangeProperties() {};
	CORE_API virtual void ProcessSerialArchive(class Archive* Arch);
	CORE_API virtual void EditorUpdate() {};
protected:
	int TypeID = -1;
	void Internal_SetOwner(GameObject* ptr);
	bool DoesUpdate = true;
	bool DoesFixedUpdate = false;
	class Scene* GetOwnerScene();
private:
	GameObject* Owner = nullptr;	
	friend GameObject;
};

