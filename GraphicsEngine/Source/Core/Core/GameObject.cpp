#include "GameObject.h"
#include "Physics/GenericRigidBody.h"
#include "RHI/RHI.h"
#include "Components\MeshRendererComponent.h"
#include "Core/Assets/Scene.h"
#include "Assets\SceneJSerialiser.h"
#include "include\glm\gtx\quaternion.hpp"
#include "Components\CompoenentRegistry.h"
#include "Core/Assets/Archive.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Core/Platform/PlatformCore.h"
GameObject::GameObject(std::string name, EMoblity stat, int oid)
{
	Name = name;
	ObjectID = oid;
	Mobilty = stat;
	m_transform = new Transform();
}

Material* GameObject::GetMat()
{
	if (m_MeshRenderer != nullptr)
	{
		return m_MeshRenderer->GetMaterial();
	}
	return nullptr;
}

GameObject::~GameObject()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		delete m_Components[i];
	}
	delete m_transform;
}

Transform * GameObject::GetTransform()
{
	return m_transform;
}

Scene * GameObject::GetScene()
{
	return OwnerScene;
}
void GameObject::Internal_SetScene(Scene * scene)
{
	OwnerScene = scene;
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->SceneInitComponent();
	}
	PositionDummy = GetTransform()->GetPos();
}

Mesh * GameObject::GetMesh()
{
	if (m_MeshRenderer != nullptr)
	{
		return m_MeshRenderer->GetMesh();
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
Component* GameObject::AttachComponent(Component * Component)
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
	Component->Internal_SetOwner(this);
	return Component;
}

std::vector<Component*> GameObject::GetComponents()
{
	return m_Components;
}

void GameObject::CopyPtrs(GameObject *)
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		//Component* comp = new Component(m_Components[i]);
		//todo Copy ptrs!
	}
}

void GameObject::SetParent(GameObject * Parent)
{
	mParent = Parent;
	GetTransform()->SetParent(mParent->GetTransform());
}
#if WITH_EDITOR
std::vector<Inspector::InspectorProperyGroup> GameObject::GetInspectorFields()
{
	std::vector<Inspector::InspectorProperyGroup> test;
	Inspector::InspectorProperyGroup RootGroup = Inspector::CreatePropertyGroup("GameObject");
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

void GameObject::MoveComponent(glm::vec3 newpos, glm::quat newrot, bool UpdatePhysics)
{
	GetTransform()->SetPos(newpos);
	GetTransform()->SetQrot(newrot);
	if (PhysicsBodyComponent != nullptr && UpdatePhysics)
	{
		PhysicsBodyComponent->MovePhysicsBody(newpos, newrot);
	}
}
