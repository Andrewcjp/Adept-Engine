#pragma once
#include "Rendering/Core/Mesh.h"
#include "Transform.h"
#include "Rendering/Core/Material.h"
#include "Editor/IInspectable.h"
class Component;
class MeshRendererComponent;
class GameObject 
#if WITH_EDITOR
	: public IInspectable
#endif
{
public:

	enum EMoblity { Static, Dynamic };
	CORE_API GameObject(std::string name = "", EMoblity stat = EMoblity::Static, int ObjectID = -1);
	CORE_API ~GameObject();

	CORE_API Transform* GetTransform();
	CORE_API class Scene* GetScene();
	void Internal_SetScene(Scene* scene);

	//Update
	CORE_API void FixedUpdate(float delta);
	CORE_API void Update(float delta);
	CORE_API void BeginPlay();

	void Render(bool ignoremat, RHICommandList* list);

	EMoblity GetMobility();
	Mesh* GetMesh();
	Material* GetMat();
	std::string GetName() { return Name; }

	void SetName(std::string name) { Name = name; }
	bool HasCached = false;

	
#if WITH_EDITOR
	//Editor only
	void EditorUpdate();
#endif
	//Components
	template<class T>
	T* GetComponent();
	template<class T>
	std::vector<T*> GetAllComponentsOfType();
	CORE_API Component* AttachComponent(Component* Component);
	std::vector<Component*> GetComponents();

	void CopyPtrs(GameObject* newObject);
	CORE_API void SetParent(GameObject* Parent);
	void ProcessSerialArchive(class Archive* Arch);

#if WITH_EDITOR
	void PostChangeProperties();
#endif
	void ChangePos_editor(glm::vec3 NewPos);

	//Movement
	CORE_API void SetPosition(glm::vec3 newpos);
	CORE_API void SetRotation(glm::quat newrot);
	void MoveComponent(glm::vec3 newpos, glm::quat newrot, bool UpdatePhysics = true);
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
private:

	//all object created from scene will have id 
	//other wise -1 is value for non scene objects 
	int ObjectID = 0;
	std::string Name = "";
	glm::vec3 PositionDummy;

	Transform* m_transform = nullptr;
	EMoblity Mobilty = EMoblity::Static;
	std::vector<Component*> m_Components;
	MeshRendererComponent* m_MeshRenderer = nullptr;
	GameObject* mParent = nullptr;
	std::vector<GameObject*> Children;
#if WITH_EDITOR
	virtual std::vector<Inspector::InspectorProperyGroup> GetInspectorFields() override;
#endif
	Scene* OwnerScene;
	class RigidbodyComponent* PhysicsBodyComponent = nullptr;
};

template<class T>
inline T * GameObject::GetComponent()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		T* Target = dynamic_cast<T*>(m_Components[i]);
		if (Target != nullptr)
		{
			return Target;
		}
	}
	return nullptr;
}

template<class T>
inline std::vector<T*> GameObject::GetAllComponentsOfType()
{
	std::vector<T*> Comps;
	for (int i = 0; i < m_Components.size(); i++)
	{
		T* Target = dynamic_cast<T*>(m_Components[i]);
		if (Target != nullptr)
		{
			Comps.push_back(Target);
		}
	}
	return Comps;
}