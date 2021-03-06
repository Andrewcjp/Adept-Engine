#pragma once
#include "Rendering/Core/Mesh.h"
#include "Transform.h"
#if WITH_EDITOR
#include "Editor/IEdtiorInspectable.h"
#endif
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
#include "EngineTypes.h"
#include "Types/WeakObjectPtr.h"
#include "Reflection/IReflect.h"
class Component;
class MeshRendererComponent;
UCLASS()
class GameObject :
	public IGarbageCollectable<GameObject>,public IReflect
#if WITH_EDITOR
	, public IEdtiorInspectable
#endif
{
public:
	CLASS_BODY_Reflect();
	TagContainer Tags;
	enum EMoblity
	{
		Static, Dynamic
	};
	CORE_API GameObject(std::string name = "", EMoblity stat = EMoblity::Static, int ObjectID = -1);
	CORE_API ~GameObject();
	CORE_API static GameObject* Instantiate(glm::vec3 Pos, glm::quat Rotation = glm::quat());
	CORE_API static void FinishGameObjectSpawn(GameObject * Object);
	CORE_API Transform* GetTransform();
	CORE_API class Scene* GetScene();
	void SetMoblity(EMoblity state);
	void Internal_SetScene(Scene* scene);
	CORE_API void Destory();
	//Update
	CORE_API void FixedUpdate(float delta);
	CORE_API void Update(float delta);
	CORE_API void BeginPlay();
	CORE_API float GetMass();
	void PrepareDataForRender();
	CORE_API EMoblity GetMobility();
	CORE_API Mesh* GetMesh();
	CORE_API MeshRendererComponent* GetMeshRenderer() const;
	CORE_API std::string GetName();
	CORE_API RigidBody* GetRigidbody();
	CORE_API void SetName(std::string name);

	
#if WITH_EDITOR
	//Editor only
	void EditorUpdate();
#endif
	//Components
	template<class T>
	T* GetComponent();
	template<class T>
	std::vector<T*> GetAllComponentsOfType();
	template<class T>
	T * GetComponentInChildren();
	template<class T>
	T* AttachComponent(T*Component)
	{
		return (T*)IN_AttachComponent(Component);
	}

	std::vector<Component*> GetComponents();
	CORE_API void SetParent(GameObject* Parent);

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
	void BroadCast_OnCollide(CollisonData Data);
	void BroadCast_OnTrigger(CollisonData Data);
	int GetAudioId();
	void OnRemoveFromScene();
	void ValidateObjectInWorld();
	CullingAABB* GetBounds();
	bool IsCulled(ECullingPass::Type Pass);
	void SetCulledState(ECullingPass::Type pass, bool state);
	bool IsStatic() const;
	SceneLayerMask LayerMask;
	bool IsOnLayer(SceneLayerMask Mask)const;

	uint GetLastMovedFrame();
	bool DisableCulling = false;
	bool GetIsActive() const { return IsActive; }
	void SetIsActive(bool val) { IsActive = val; }

	void Serialize(BinaryArchive* Achive) override;

private:
	std::vector<ClassReflectionNode*> TransformNodes;
	bool IsActive = true;
	bool IsDead = false;
	CORE_API Component* IN_AttachComponent(Component* Component);
	//all object created from scene will have id 
	//other wise -1 is value for non scene objects 
	int ObjectID = 0;
	PROPERTY();
	std::string Name = "";
	glm::vec3 PositionDummy;

	Transform* m_transform = nullptr;
	EMoblity Mobilty = EMoblity::Static;
	std::vector<Component*> m_Components;
	MeshRendererComponent* m_MeshRenderer = nullptr;
	GameObject* mParent = nullptr;
	std::vector<GameObject*> Children;
#if WITH_EDITOR
	virtual std::vector<InspectorProperyGroup> GetInspectorFields() override;

#endif
	Scene* OwnerScene = nullptr;
	class RigidbodyComponent* PhysicsBodyComponent = nullptr;
	class ColliderComponent* PhyscsCollider = nullptr;
	int AudioId = 0;
	bool CullingStates[ECullingPass::Limit] = { false,false };
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

template<class T>
inline T* GameObject::GetComponentInChildren()
{
	for (int i = 0; i < Children.size(); i++)
	{
		T* Target = Children[i]->GetComponent<T>();
		if (Target != nullptr)
		{
			return Target;
		}
	}
	return nullptr;
}