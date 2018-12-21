#include "GameObject.h"
#include "Assets/Archive.h"
#include "Assets/Scene.h"
#include "Audio/AudioEngine.h"
#include "BaseWindow.h"
#include "Components/MeshRendererComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Platform/PlatformCore.h"
#include "Components/ColliderComponent.h"


GameObject::GameObject(std::string name, EMoblity stat, int oid)
{
	Name = name;
	ObjectID = oid;
	Mobilty = stat;
	m_transform = new Transform();
	AudioId = AudioEngine::GetNextAudioId();
	AudioEngine::RegisterObject(this);
}

void GameObject::OnRemoveFromScene()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->OnDestroy();
	}
}

void GameObject::ValidateObjectInWorld()
{
	const glm::vec3 WorldBounds = glm::vec3(10000);
	if (GetPosition().x > WorldBounds.x || GetPosition().y > WorldBounds.y || GetPosition().z > WorldBounds.z ||
		GetPosition().x < -WorldBounds.x || GetPosition().y < -WorldBounds.y || GetPosition().z < -WorldBounds.z)
	{
		Destory();
	}
}

GameObject::~GameObject()
{
	AudioEngine::DeRegisterObject(this);
	for (int i = 0; i < m_Components.size(); i++)
	{
		SafeDelete(m_Components[i]);
	}
	SafeDelete(m_transform);
}

GameObject * GameObject::Instantiate(glm::vec3 Pos, glm::quat Rotation)
{
	GameObject* NewObject = new GameObject();
	NewObject->SetPosition(Pos);
	NewObject->SetRotation(Rotation);
	return NewObject;
}

void GameObject::FinishGameObjectSpawn(GameObject* Object)
{
	BaseWindow::GetScene()->AddGameobjectToScene(Object);
}

Transform * GameObject::GetTransform()
{
#if !USE_TRANSFORM_CACHING
	m_transform->GetModel();
#endif
	return m_transform;
}

Scene * GameObject::GetScene()
{
	return OwnerScene;
}

void GameObject::Internal_SetScene(Scene * scene)
{
	OwnerScene = scene;
	if (scene != nullptr)
	{
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i]->SceneInitComponent();
		}
		PositionDummy = GetTransform()->GetPos();
	}
}

void GameObject::Destory()
{
	if (GetScene() && !IsDead)
	{
		GetScene()->RemoveGameObject(this);
		IsDead = true;
	}
}

Mesh * GameObject::GetMesh()
{
	if (m_MeshRenderer != nullptr)
	{
		return m_MeshRenderer->GetMesh();
	}
	return nullptr;
}

RigidBody * GameObject::GetRigidbody()
{
	if (PhysicsBodyComponent != nullptr)
	{
		return PhysicsBodyComponent->GetActor();
	}
	return nullptr;
}

void GameObject::Render(bool ignoremat, RHICommandList * list)
{
	if (m_MeshRenderer != nullptr)
	{
		m_MeshRenderer->Render(ignoremat, list);
	}
}
void GameObject::FixedUpdate(float delta)
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		if (m_Components[i]->DoesFixedUpdate)
		{
			m_Components[i]->FixedUpdate(delta);
		}
	}
}

void GameObject::Update(float delta)
{
	bool changed = m_transform->IsChanged();
	for (int i = 0; i < m_Components.size(); i++)
	{
		if (m_Components[i]->DoesUpdate)
		{
			m_Components[i]->Update(delta);
			if (changed)
			{
				m_Components[i]->OnTransformUpdate();//todo:optmize Transform move!
			}
		}
	}
	if (changed)
	{
		m_transform->Update();
	}
	AudioEngine::UpdateWiseTransfrom(this);
	ValidateObjectInWorld();
}

void GameObject::BeginPlay()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->BeginPlay();
	}
}

GameObject::EMoblity GameObject::GetMobility()
{
	return Mobilty;
}
#if WITH_EDITOR
void GameObject::EditorUpdate()
{

	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->EditorUpdate();
	}

	bool changed = m_transform->IsChanged();
	if (changed)
	{
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i]->OnTransformUpdate();
		}
		//GetScene()->StaticSceneNeedsUpdate = true;
		m_transform->Update();
	}
}
#endif
Component* GameObject::IN_AttachComponent(Component * Component)
{
	if (Component == nullptr)
	{
		return nullptr;
	}
	MeshRendererComponent* Renderer = dynamic_cast<MeshRendererComponent*>(Component);
	if (Renderer != nullptr)
	{
		m_MeshRenderer = Renderer;
	}
	if (Component != nullptr)
	{
		m_Components.push_back(Component);
	}

	RigidbodyComponent* NewRigidbody = dynamic_cast<RigidbodyComponent*>(Component);
	if (NewRigidbody != nullptr)
	{
		ensure(PhysicsBodyComponent == nullptr);
		PhysicsBodyComponent = NewRigidbody;
	}
	ColliderComponent* NewCollider = dynamic_cast<ColliderComponent*>(Component);
	if (NewCollider != nullptr)
	{
		//ensure(PhyscsCollider == nullptr);
		PhyscsCollider = NewCollider;
	}
	Component->Internal_SetOwner(this);
	if (OwnerScene != nullptr)
	{
		Component->SceneInitComponent();
	}
	return Component;
}

std::vector<Component*> GameObject::GetComponents()
{
	return m_Components;
}

void GameObject::SetParent(GameObject * Parent)
{
	mParent = Parent;
	GetTransform()->SetParent(mParent);
	if (mParent != nullptr)
	{
		mParent->Children.push_back(this);
	}	
}
#if WITH_EDITOR
std::vector<InspectorProperyGroup> GameObject::GetInspectorFields()
{
	std::vector<InspectorProperyGroup> test;
	InspectorProperyGroup RootGroup = Inspector::CreatePropertyGroup("GameObject");
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Name", EditValueType::String, &Name));
	test.push_back(RootGroup);
	RootGroup = Inspector::CreatePropertyGroup("Transform");
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Position x", EditValueType::Float, &PositionDummy.x));
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Position y", EditValueType::Float, &PositionDummy.y));
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Position z", EditValueType::Float, &PositionDummy.z));
	test.push_back(RootGroup);
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->GetInspectorProps(test);
	}
	return test;
}
#endif

void GameObject::ProcessSerialArchive(Archive* A)
{
	ArchiveProp(GetTransform());
	ArchiveProp(Name);
	if (A->IsReading())
	{
		std::vector<Component*> CompStaging;
		ArchiveProp_Alias(CompStaging, m_Components);
		for (Component* C : CompStaging)
		{
			AttachComponent(C);
		}
	}
	else
	{
		ArchiveProp(m_Components);
	}
}
#if WITH_EDITOR
//called when the editor updates a value
void GameObject::PostChangeProperties()
{
	GetScene()->StaticSceneNeedsUpdate = true;
	GetTransform()->SetPos(PositionDummy);
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->PostChangeProperties();
	}
}
#endif
void GameObject::ChangePos_editor(glm::vec3 NewPos)
{
	PositionDummy = NewPos;
}

void GameObject::SetPosition(glm::vec3 newpos)
{
	MoveComponent(newpos, GetRotation());
}

void GameObject::SetRotation(glm::quat newrot)
{
	MoveComponent(GetPosition(), newrot);
}

glm::vec3 GameObject::GetPosition()
{
	return GetTransform()->GetPos();
}

glm::quat GameObject::GetRotation()
{
	return GetTransform()->GetQuatRot();
}

void GameObject::BroadCast_OnCollide(CollisonData Data)
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->OnCollide(Data);
	}
}
void GameObject::BroadCast_OnTrigger(CollisonData Data)
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->OnTrigger(Data);
	}
}

void GameObject::MoveComponent(glm::vec3 newpos, glm::quat newrot, bool UpdatePhysics)
{
	GetTransform()->SetPos(newpos);
	GetTransform()->SetQrot(newrot);
	if (UpdatePhysics)
	{
		if (PhysicsBodyComponent != nullptr)
		{
			PhysicsBodyComponent->MovePhysicsBody(newpos, newrot);
		}
		else if (PhyscsCollider != nullptr)
		{
			PhyscsCollider->MovePhysicsBody(newpos, newrot);
		}
	}
}

float GameObject::GetMass()
{
	if (PhysicsBodyComponent != nullptr)
	{
		return PhysicsBodyComponent->GetActor()->GetMass();
	}
	return 1.0f;
}
