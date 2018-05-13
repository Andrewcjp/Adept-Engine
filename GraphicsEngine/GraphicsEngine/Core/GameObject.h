#pragma once
#include "../Rendering/Core/Renderable.h"
#include "Transform.h"

#include "../Rendering/Core/Material.h"
#include "Editor/IInspectable.h"
#include "physx\PxShape.h"
#include "physx\PxRigidStatic.h"
#include "rapidjson\document.h"
class Component;
class RigidBody;
class MeshRendererComponent;
class GameObject :
	public IInspectable
{
public:
	enum EMoblity { Static, Dynamic };
	GameObject(std::string name = "", EMoblity stat = EMoblity::Static, int ObjectID = -1);
	~GameObject();

	inline Transform* GetTransform();
	class Scene* GetScene();
	void Internal_SetScene(Scene* scene);

	//Update
	void FixedUpdate(float delta);
	void Update(float delta);
	void BeginPlay();

	void Render(bool ignore = false);
	void Render(bool ignoremat, CommandListDef * list);
	void Render(bool ignoremat, RHICommandList* list);
	bool CheckCulled(float Distance, float angle);
	EMoblity GetMobility();

	//temp
	Renderable* GetMesh();
	RigidBody* actor;



	//getters
	Material* GetMat();
	std::string GetName() { return Name; }
	void SetName(std::string name) { Name = name; }
	bool GetReflection();	
	bool GetDoesUseMainShader();
	bool Occluded = false;

	bool QuerryWait = false;
	bool HasCached = false;

	
	//Editor only
	void EditorUpdate();
	physx::PxRigidStatic* SelectionShape;
	Component* AttachComponent(Component* Component);
	std::vector<Component*> GetComponents();
	template<class T>
	T* GetComponent();
	

	void CopyPtrs(GameObject* newObject);
	void SerialiseGameObject(rapidjson::Value& v);
	void DeserialiseGameObject(rapidjson::Value& v);
	void PostChangeProperties();
private:
	//all object created from scene will have id 
	//other wise -1 is value for non scene objects 
	int ObjectID = 0;
	std::string Name;
	glm::vec3 PositionDummy;

	Transform* m_transform = nullptr;
	EMoblity Mobilty;
	std::vector<Component*> m_Components;
	MeshRendererComponent* m_MeshRenderer = nullptr;
	GameObject* mParent = nullptr;
	std::vector<GameObject*> Children;
	virtual std::vector<Inspector::InspectorProperyGroup> GetInspectorFields() override;
	Scene* OwnerScene;
	const char * ComponentArrayKey = "Components";
};

template<class T>
inline T * GameObject::GetComponent()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		T* Target = static_cast<T>(m_Components[i]);
		if (Target != nullptr)
		{
			return Target;
		}
	}
	return nullptr;
}
