#include "TestGameGameMode.h"
#include "Core/Components/Core_Components_inc.h"
#include "TestPlayer.h"
TestGameGameMode::TestGameGameMode()
{}


TestGameGameMode::~TestGameGameMode()
{}

void TestGameGameMode::BeginPlay(Scene* Scene)
{
	GameMode::BeginPlay(Scene);

	GameObject* go = new GameObject("Player Test");
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Sphere.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 10, 10));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	go->AttachComponent(new RigidbodyComponent());
	go->AttachComponent(new ColliderComponent());
	go->AttachComponent(new TestPlayer());
	BodyInstanceData lock;
	lock.LockXRot = true;
	lock.LockYRot = true;
	lock.LockZRot = true;
	go->GetComponent<RigidbodyComponent>()->SetLockFlags(lock);
	Scene->AddGameobjectToScene(go);
}

void TestGameGameMode::EndPlay()
{

}

void TestGameGameMode::Update()
{

}
