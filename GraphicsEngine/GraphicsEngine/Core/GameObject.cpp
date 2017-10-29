#include "GameObject.h"
#include "Physics/RigidBody.h"
#include "RHI/RHI.h"
#include "Components\MeshRendererComponent.h"
GameObject::GameObject(std::string name, EMoblity stat, int oid)
{
	Name = name;

	/*void* ptr = (void*)(&Name);
	std::string* test = (std::string*)ptr;
	*test = "enjrgne4";*/
	ObjectID = oid;
	Mobilty = stat;
	m_transform = new Transform();
	//init the querry 
	if (RHI::GetType() == RenderSystemOGL)
	{
		glGenQueries(1, &Querry);
	}
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
bool  GameObject::CheckCulled(float Distance, float angle)
{
	if (Distance > 250)
	{
		return true;
	}
	if (angle > 50)
	{
		return true;
	}
	return false;
}
void GameObject::Render(bool ignoremat)
{
	/*if (m_mat != NULL && ignoremat == false)
	{
		m_mat->SetMaterialActive();
	}

	m_mesh->Render();*/
	if (m_MeshRenderer != nullptr)
	{
		m_MeshRenderer->Render(ignoremat);
	}
}

void GameObject::FixedUpdate(float delta)
{
	if (actor != nullptr)
	{
		m_transform->SetPos(actor->GetPosition());
		m_transform->SetQrot(actor->GetRotation());
	}
	if (SelectionShape != nullptr)
	{
		//SelectionShape->
	}
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
	for (int i = 0; i < m_Components.size(); i++)
	{
		if (m_Components[i]->DoesUpdate)
		{
			m_Components[i]->Update(delta);
		}
	}
}

void GameObject::BeginPlay()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->BeginPlay();
	}
}

void GameObject::AttachComponent(Component * Component)
{
	MeshRendererComponent* Renderer = static_cast<MeshRendererComponent*>(Component);
	if (Renderer != nullptr)
	{
		m_MeshRenderer = Renderer;
	}
	if (Component != nullptr)
	{
		m_Components.push_back(Component);
	}
}

std::vector<Component*> GameObject::GetComponents()
{
	return m_Components;
}

std::vector<Inspector::InspectorProperyGroup> GameObject::GetInspectorFields()
{
	std::vector<Inspector::InspectorProperyGroup> test;
	Inspector::InspectorProperyGroup RootGroup = Inspector::CreatePropertyGroup("GameObject");

	//Inspector::InspectorPropery NameProp;
	//NameProp.name = "Name";
	//NameProp.type = Inspector::String;
	//NameProp.ValuePtr = &Name;
	//test.push_back(NameProp);
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Test Float", Inspector::Float, nullptr));
	RootGroup.SubProps.push_back(Inspector::CreateProperty("Test Float", Inspector::Int, nullptr));
	test.push_back(RootGroup);
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->GetInspectorProps(test);
	}
	return test;
}
