#include "TestGameGameMode.h"
#include "Core/Components/Core_Components_inc.h"
#include "Components/TestPlayer.h"
#include "Components/Weapon.h"
#include "AI/SkullChaser.h"
#include "Components/Health.h"
#include "Components/Projectile.h"
TestGameGameMode::TestGameGameMode()
{}


TestGameGameMode::~TestGameGameMode()
{}

void TestGameGameMode::BeginPlay(Scene* Scene)
{
	GameMode::BeginPlay(Scene);
#if TDSIM_ENABLED
	return;
#endif
	GameObject* go = new GameObject("Player Test");
	player = go;
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Sphere.obj"), mat));
	go->SetPosition(glm::vec3(0, 10, 10));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	go->AttachComponent(new RigidbodyComponent());
	ColliderComponent* cc= go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	go->AttachComponent(new Weapon());
	TestPlayer* player = (TestPlayer*)go->AttachComponent(new TestPlayer());
	BodyInstanceData lock;
	lock.LockXRot = true;
	lock.LockZRot = true;
	go->GetComponent<RigidbodyComponent>()->SetLockFlags(lock);
	Scene->AddGameobjectToScene(go);
	GameObject* Cam = new GameObject("PlayerCamera");
	Cam->SetParent(go);
	Scene->AddGameobjectToScene(Cam);
	player->CameraObject = Cam;

	SpawnSKull(glm::vec3(20, 5, 0));
	SpawnSKull(glm::vec3(-15, 5, 0));
}

void TestGameGameMode::SpawnSKull(glm::vec3 Position)
{
	GameObject* skull = GameObject::Instantiate(Position);
	SkullChaser* c = skull->AttachComponent(new SkullChaser());
	c->Player = player;
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;
	skull->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Munkey.obj"), mat));
	skull->AttachComponent(new ColliderComponent());
	RigidbodyComponent* rb = skull->AttachComponent(new RigidbodyComponent());
	rb->SetGravity(false);
	Health* h = skull->AttachComponent(new Health());
	h->MaxHealth = 10.0f;
	Projectile* p = skull->AttachComponent(new Projectile());
	p->SetDamage(50);
	CurrentScene->AddGameobjectToScene(skull);
}

void TestGameGameMode::EndPlay()
{

}

void TestGameGameMode::Update()
{

}
