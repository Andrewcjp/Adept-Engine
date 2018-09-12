#include "GameObject.h"
#include "Physics/RigidBody.h"
#include "RHI/RHI.h"
#include "Components\MeshRendererComponent.h"
#include "Core/Assets/Scene.h"
#include "Assets\SceneJSerialiser.h"
#include "include\glm\gtx\quaternion.hpp"
#include "Components\CompoenentRegistry.h"
#include "Core/Assets/Archive.h"
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

bool GameObject::GetReflection()
{
	if (m_MeshRenderer != nullptr)
	{
		return m_MeshRenderer->GetMaterial()->GetProperties()->IsReflective;
	}
	return false;
}

bool GameObject::GetDoesUseMainShader()
{
	if (m_MeshRenderer != nullptr)
	{
		return m_MeshRenderer->GetMaterial()->GetProperties()->UseMainShader;
	}
	return false;
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
	if (SelectionShape != nullptr)
	{
		//SelectionShape->
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

void GameObject::EditorUpdate()
{
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

std::vector<Inspector::InspectorProperyGroup> GameObject::GetInspectorFields()
{
	std::vector<Inspector::InspectorProperyGroup> test;
	Inspector::InspectorProperyGroup RootGroup = Inspector::CreatePropertyGroup("GameObject");
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Name", Inspector::String, &Name));
	test.push_back(RootGroup);
	RootGroup = Inspector::CreatePropertyGroup("Transform");
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Position x", Inspector::Float, &PositionDummy.x));
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Position y", Inspector::Float, &PositionDummy.y));
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Position z", Inspector::Float, &PositionDummy.z));
	test.push_back(RootGroup);
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->GetInspectorProps(test);
	}
	return test;
}


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

void GameObject::ChangePos_editor(glm::vec3 NewPos)
{
	PositionDummy = NewPos;
}
