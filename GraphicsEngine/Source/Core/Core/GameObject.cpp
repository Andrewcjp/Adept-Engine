#include "GameObject.h"
#include "Assets/Archive.h"
#include "Assets/Scene.h"
#include "Audio/AudioEngine.h"
#include "BaseWindow.h"
#include "Components/MeshRendererComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Platform/PlatformCore.h"
#include "Components/ColliderComponent.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/Mesh/MeshPipelineController.h"
#include "Assets/BinaryArchive.h"
#include "Components/ComponentRegistry.h"
#include "Reflection/ObjectLibrary.h"
#include "Core/GameObject.generated.h"

GameObject::GameObject(std::string name, EMoblity stat, int oid) :
	Name(name)
{

	ObjectID = oid;
	Mobilty = stat;
	m_transform = new Transform();
	ClassReflectionNode* TransLate = new ClassReflectionNode("Translation", MemberValueType::Vector3, new glm::vec3());
	TransLate->MapPropToFunctions([&](void* pos) { *(glm::vec3*)pos = GetPosition(); }, [&](void* pos) { SetPosition(*(glm::vec3*)pos); });
	TransformNodes.push_back(TransLate);
	LayerMask.SetFlags(0xfffff);
	CALL_CONSTRUCTOR();
}

void GameObject::OnRemoveFromScene()
{
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->OnDestroy();
	}
	SceneRenderer::Get()->GetPipelineController()->RemoveBatches(this);
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

CullingAABB * GameObject::GetBounds()
{
	if (GetMesh() != nullptr)
	{
		return GetMesh()->GetBounds();
	}
	return nullptr;
}

bool GameObject::IsCulled(ECullingPass::Type Pass)
{
	return CullingStates[Pass];
}

void GameObject::SetCulledState(ECullingPass::Type pass, bool state)
{
	CullingStates[pass] = state;
}

bool GameObject::IsStatic() const
{
	return Mobilty == EMoblity::Static;
}

bool GameObject::IsOnLayer(SceneLayerMask Mask) const
{
	for (int i = 0; i < ESceneLayers::Limit; i++)
	{
		int flag = 1 << i;
		if (!Mask.GetFlagValue(flag))
		{
			continue;
		}
		if (Mask.GetFlagValue(flag) == LayerMask.GetFlagValue(flag))
		{
			return true;
		}
	}
	return false;
}

uint GameObject::GetLastMovedFrame()
{
#if !WITH_EDITOR
	if (IsStatic())
	{
		//todo: push transfrom once
		return 0;
	}
#endif
	return GetTransform()->GetLastMovedFrame();
}

void GameObject::Serialize(BinaryArchive* Achive)
{
	SerializeThis(Achive, m_RelfectionData.Data);
	SerializeThis(Achive, TransformNodes);
	int Length = m_Components.size();
	Achive->LinkHeader(Length);

	int* ComponentIds = new int[Length];
	if (!Achive->Reading)
	{
		for (int i = 0; i < Length; i++)
		{
			ComponentIds[i] = m_Components[i]->GetId();
		}
	}
	Achive->LinkData(ComponentIds, sizeof(int)*Length);
	if (Achive->Reading)
	{
		for (int i = 0; i < Length; i++)
		{
			Component* c = ObjectLibrary::Create<Component>(ComponentIds[i]);
			c->Serialize(Achive);
			AttachComponent(c);
		}
	}
	else
	{
		for (int i = 0; i < Length; i++)
		{
			m_Components[i]->Serialize(Achive);
		}
	}
	delete[] ComponentIds;
}

GameObject::~GameObject()
{
	AudioEngine::DeRegisterObject(this);
	MemoryUtils::DeleteVector(m_Components);
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

void GameObject::SetMoblity(EMoblity state)
{
	Mobilty = state;
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
		for (int i = 0; i < Children.size(); i++)
		{
			Children[i]->Destory();
		}
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

MeshRendererComponent * GameObject::GetMeshRenderer() const
{
	return m_MeshRenderer;
}

std::string GameObject::GetName()
{
	return Name;
}

RigidBody * GameObject::GetRigidbody()
{
	if (PhysicsBodyComponent != nullptr)
	{
		return PhysicsBodyComponent->GetActor();
	}
	return nullptr;
}


void GameObject::PrepareDataForRender()
{
	if (m_MeshRenderer != nullptr)
	{
		m_MeshRenderer->PrepareDataForRender();
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
				m_Components[i]->OnTransformUpdate();//#Transform :optimize Transform move!
			}
		}
	}
	if (changed)
	{
		//m_transform->Update();
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

void GameObject::SetName(std::string name)
{
	Name = name;
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
	RootGroup.Nodes = AccessReflection()->Data;
	test.push_back(RootGroup);
	RootGroup = Inspector::CreatePropertyGroup("Transform");
	RootGroup.Nodes = TransformNodes;
	test.push_back(RootGroup);

	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->GetInspectorProps(test);
	}
	return test;
}
#endif

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
	SetPosition(NewPos);
	for (int i = 0; i < m_Components.size(); i++)
	{
		m_Components[i]->OnTransformUpdate();
	}
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

int GameObject::GetAudioId()
{
	return AudioId;
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
		//		return PhysicsBodyComponent->GetActor()->GetMass();
	}
	return 1.0f;
}
