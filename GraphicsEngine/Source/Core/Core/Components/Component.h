#pragma once
#include "Editor/EditorTypes.h"
#include "Physics/PhysicsTypes.h"
#include "Core/Reflection/ClassReflectionNode.h"
#include "Core/Reflection/IReflect.h"
struct InspectorProperyGroup;
class Scene;
class GameObject;
class Component : public IReflect
{
public:
	CORE_API Component();
	CORE_API virtual ~Component();
	CORE_API virtual void InitComponent()
	{};
	CORE_API virtual void BeginPlay();
	CORE_API virtual void Update(float delta);
	CORE_API virtual void FixedUpdate(float delta);
	CORE_API GameObject* GetOwner();
	CORE_API virtual void OnTransformUpdate();
	CORE_API virtual void SceneInitComponent()
	{};
	CORE_API virtual void ProcessSerialArchive(class Archive* Arch);

#if WITH_EDITOR
	CORE_API virtual void EditorUpdate()
	{};
	CORE_API virtual void PostChangeProperties()
	{};
	CORE_API virtual void GetInspectorProps(std::vector<InspectorProperyGroup> &props);
#endif

	CORE_API virtual void OnCollide(CollisonData data)
	{};
	CORE_API virtual void OnTrigger(CollisonData data)
	{};
	CORE_API virtual void OnDestroy();
	
protected:
	std::string m_DisplayName = "Component";
	int TypeID = -1;
	void Internal_SetOwner(GameObject* ptr);
	bool DoesUpdate = true;
	bool DoesFixedUpdate = false;
	CORE_API Scene* GetOwnerScene();
private:
	GameObject* Owner = nullptr;
	friend GameObject;
};

