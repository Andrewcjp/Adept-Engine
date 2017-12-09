#include "GameObject.h"
#include "Physics/RigidBody.h"
#include "RHI/RHI.h"
#include "Components\MeshRendererComponent.h"
#include "../Core/Assets/Scene.h"
//#include "Assets\SerialHelpers.h"
#include "Assets\SceneJSerialiser.h"
#include "include\glm\gtx\quaternion.hpp"
#include "Components\CompoenentRegistry.h"
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
Scene * GameObject::GetScene()
{
	return OwnerScene;
}
void GameObject::Internal_SetScene(Scene * scene)
{
	OwnerScene = scene;
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

//temp

Renderable * GameObject::GetMesh()
{
	if (m_MeshRenderer != nullptr)
	{
		return m_MeshRenderer->GetMesh();
	}
	return nullptr;
}
void GameObject::Render(bool ignoremat)
{
	if (m_MeshRenderer != nullptr)
	{
		m_MeshRenderer->Render(ignoremat, nullptr);
	}
}
void GameObject::Render(bool ignoremat, CommandListDef* list)
{
	
	if (m_MeshRenderer != nullptr)
	{
		m_MeshRenderer->Render(ignoremat, list);
	}
}
void GameObject::FixedUpdate(float delta)
{
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

void GameObject::EditorUpdate()
{
	bool changed = m_transform->IsChanged();
	if (changed)
	{
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i]->OnTransformUpdate();
		}
		GetScene()->StaticSceneNeedsUpdate = true;
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

void GameObject::CopyPtrs(GameObject * newObject)
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

void GameObject::SerialiseGameObject(rapidjson::Value& v)
{
	SerialHelpers::addString(v, *SceneJSerialiser::jallocator, "Name", Name);
	SerialHelpers::addVector(v, *SceneJSerialiser::jallocator, "Pos", GetTransform()->GetPos());
	SerialHelpers::addVector(v, *SceneJSerialiser::jallocator, "Rot", glm::eulerAngles(GetTransform()->GetQuatRot()));
	SerialHelpers::addVector(v, *SceneJSerialiser::jallocator, "Scale", GetTransform()->GetScale());
	rapidjson::Value comp(rapidjson::kArrayType);
	for (int i = 0; i < m_Components.size(); i++)
	{
		rapidjson::Value jsv(rapidjson::kObjectType);
		m_Components[i]->Serialise(jsv);
		comp.PushBack(jsv, *SceneJSerialiser::jallocator);
	}
	SerialHelpers::addJsonValue(v, *SceneJSerialiser::jallocator, ComponentArrayKey, comp);
}

void GameObject::DeserialiseGameObject(rapidjson::Value & v)
{
	for (auto& it = v.MemberBegin(); it != v.MemberEnd(); it++)
	{
		std::string key = (it->name.GetString());
		if (key == "Name")
		{
			SetName(it->value.GetString());
		}
		if (key == "Pos")
		{
			glm::vec3 pos;
			if (SerialHelpers::getFloatVec<3>(it->value, "Pos", &pos[0]))
			{
				GetTransform()->SetPos(pos);
			}
		}
		if (key == "Rot")
		{
			glm::vec3 rot;
			if (SerialHelpers::getFloatVec<3>(it->value, "Rot", &rot[0]))
			{
				glm::quat newrot = glm::toQuat(glm::orientate3(rot));
				GetTransform()->SetQrot(newrot);
			}
		}
		if (key == "Scale")
		{
			glm::vec3 scale;
			if (SerialHelpers::getFloatVec<3>(it->value, "Scale", &scale[0]))
			{
				GetTransform()->SetScale(scale);
			}
		}
		if (key == "Components")
		{
			//foreach component
			auto t = it->value.GetArray();
			t.begin();
			for (unsigned int i = 0; i < t.Size(); i++)
			{
				Component* newc = nullptr;
				rapidjson::Value*  cv = &t[i];
				for (auto& cit = cv->MemberBegin(); cit != cv->MemberEnd(); cit++)
				{
					//read the first part of the object for the components ID
					if (cit->name == "Type")
					{
						newc = CompoenentRegistry::CreateBaseComponent((CompoenentRegistry::BaseComponentTypes)cit->value.GetInt());
					}
				}
				if (newc != nullptr)
				{
					newc->Deserialise(*cv);
					AttachComponent(newc);
				}
			}
		}
	}
}
