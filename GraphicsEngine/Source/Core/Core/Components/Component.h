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
#if WITH_EDITOR
	CORE_API virtual void Component::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props);
#endif
	CORE_API class GameObject* GetOwner();

	CORE_API virtual void OnTransformUpdate();
	CORE_API virtual void SceneInitComponent() {};
#if WITH_EDITOR
	CORE_API virtual void PostChangeProperties() {};
#endif
	CORE_API virtual void ProcessSerialArchive(class Archive* Arch);
#if WITH_EDITOR
	CORE_API virtual void EditorUpdate() {};
#endif
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

