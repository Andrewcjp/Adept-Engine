#include "PhysicsThrowerComponent.h"
#include "EngineHeader.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Components/RigidbodyComponent.h"
PhysicsThrowerComponent::PhysicsThrowerComponent()
{}

PhysicsThrowerComponent::~PhysicsThrowerComponent()
{}
void PhysicsThrowerComponent::BeginPlay()
{}

void PhysicsThrowerComponent::Update(float delta)
{
	if (Input::GetKeyDown(VK_SPACE))
	{
		FireAtScene();
	}
	if (Input::GetKeyDown(VK_DOWN))
	{
		//CreateStackAtPoint();
	}
}

void PhysicsThrowerComponent::EditorUpdate()
{	
	//Todo: Debug
	Update(0);
}

void PhysicsThrowerComponent::CreateStackAtPoint()
{
	RayHit hit;
	//get main cam
	//todo Gameobject Shortcuts for transform ops
	Camera* cam = CameraComponent::GetMainCamera();
	if (cam == nullptr)
	{
		return;
	}
	if (Engine::GetPhysEngineInstance()->RayCastScene(cam->GetPosition(), cam->GetForward(), 100, &hit) == false)
	{
		return;
	}
#if 0
	std::vector<RigidBody*> objs = Engine::GetPhysEngineInstance()->createStack(hit.position, 5, 0.5);

	for (size_t i = 0; i < objs.size(); i++)
	{
		GameObject* go = new GameObject();
		Material* mat = new Material(RHI::CreateTexture(std::string("bricks2.jpg")));

		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("cubeuv.obj"), mat));
		go->GetTransform()->SetPos(objs[i]->GetPosition());
		float scale = 0.5;
		go->GetTransform()->SetScale(glm::vec3(scale));
		go->actor = objs[i];
		GetOwner()->GetScene()->AddGameobjectToScene(go);

	}
#endif
}

void PhysicsThrowerComponent::FireAtScene()
{
	Camera* cam = CameraComponent::GetMainCamera();
	if (cam == nullptr)
	{
		return;
	}
	GameObject* go = new GameObject();
	//Material* mat = new Material(RHI::CreateTexture(std::string("bricks2.jpg")));
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("TherealSpherer.obj"), mat));
	go->GetTransform()->SetPos(cam->GetPosition());
	float scale = 0.5;
	go->GetTransform()->SetScale(glm::vec3(scale));
	RigidbodyComponent* rb = new RigidbodyComponent();
	go->AttachComponent(rb);
	//rb-> = Engine::GetPhysEngineInstance()->FirePrimitiveAtScene(cam->GetPosition() + cam->GetForward() * 2, cam->GetForward() * CurrentForce, scale);
	GetOwner()->GetScene()->AddGameobjectToScene(go);
}
void PhysicsThrowerComponent::InitComponent()
{

}
