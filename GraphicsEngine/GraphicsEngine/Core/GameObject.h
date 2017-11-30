#pragma once
#include "../Rendering/Core/Renderable.h"
#include "Transform.h"
#include "GLEW\GL\glew.h"
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

	inline Transform* GetTransform()
	{
		return m_transform;
	}
	class Scene* GetScene();
	void Internal_SetScene(Scene* scene);



	[[deprecated("Use Mesh Comps")]]
	inline void SetMesh(Renderable* newmesh)
	{
		m_mesh = newmesh;
	}

	//Update
	void FixedUpdate(float delta);
	void Update(float delta);
	void BeginPlay();
	//todo: Depricate
	void Render(bool ignore = false);
	void Render(bool ignoremat, CommandListDef * list);
	bool CheckCulled(float Distance, float angle);
	//temp
	Renderable* GetMesh();

	//getters
	[[deprecated("Use Mesh Comps")]]
	void SetMaterial(Material* m)
	{
		m_mat = m;
	}
	Material* GetMat();

	std::string GetName() { return Name; }
	void SetName(std::string name) { Name = name; }
	bool GetReflection() { return IsReflective; }
	void SetReflection(bool value) { IsReflective = value; }
	bool UseDefaultShader = true;
	bool Occluded = false;
	GLuint Querry;
	bool QuerryWait = false;
	EMoblity GetMobility()
	{
		return Mobilty;
	}
	bool HasCached = false;
	RigidBody* actor;
	void EditorUpdate();
	//Editor only
	physx::PxRigidStatic* SelectionShape;
	Component* AttachComponent(Component* Component);
	std::vector<Component*> GetComponents();
	template<class T>
	T* GetComponent();
	void CopyPtrs(GameObject* newObject);
	void SerialiseGameObject(rapidjson::Value& v);
	void DeserialiseGameObject(rapidjson::Value& v);
private:
	//all object created from scene will have id 
	//other wise -1 is value for non scene objects 
	int ObjectID = 0;
	std::string Name;
	float OtherData = 0;
	Renderable* m_mesh = nullptr;
	Transform* m_transform = nullptr;
	Material* m_mat = nullptr;
	bool IsReflective = false;
	EMoblity Mobilty;
	std::vector<Component*> m_Components;
	MeshRendererComponent* m_MeshRenderer = nullptr;
	GameObject* mParent = nullptr;
	std::vector<GameObject*> Children;
	// Inherited via IInspectable
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
