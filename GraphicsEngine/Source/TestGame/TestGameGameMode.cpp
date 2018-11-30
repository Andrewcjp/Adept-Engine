#include "TestGameGameMode.h"
#include "AI/Core/AIController.h"
#include "AI/Core/AISystem.h"
#include "AI/SkullChaser.h"
#include "AI/TestGame_Director.h"
#include "Components/Health.h"
#include "Components/MeleeWeapon.h"
#include "Components/Pickup.h"
#include "Components/Projectile.h"
#include "Components/Railgun.h"
#include "Components/SpawningPool.h"
#include "Components/TestPlayer.h"
#include "Components/Weapon.h"
#include "Components/WeaponManager.h"
#include "Core/Components/Core_Components_inc.h"
#include "Physics/GenericConstraint.h"

TestGameGameMode::TestGameGameMode()
{}

TestGameGameMode::~TestGameGameMode()
{}

GameObject* MakeTestSphere(Scene* Scene)
{
	GameObject* go = Scene::CreateDebugSphere(nullptr);
	go->AttachComponent(new RigidbodyComponent());
	ColliderComponent* cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	Scene->AddGameobjectToScene(go);
	return go;
}

void TestGameGameMode::BeginPlay(Scene* Scene)
{
	GameMode::BeginPlay(Scene);
#if 0
	GameObject* A = MakeTestSphere(Scene);
	GameObject* B = MakeTestSphere(Scene);
	A->SetPosition(glm::vec3(0, 20, 0));
	ConstaintSetup data;
	ConstraintInstance* aint = Engine::GetPhysEngineInstance()->CreateConstraint(A->GetComponent<RigidbodyComponent>()->GetActor(), B->GetComponent<RigidbodyComponent>()->GetActor(), data);
#endif
#if TDSIM_ENABLED
	return;
#endif

	GameObject* go = new GameObject("Player Test");
	player = go;
	player->Tags.Add("player");
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\Sphere.obj"), mat));
	go->SetPosition(glm::vec3(0, 10, 10));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	go->AttachComponent(new RigidbodyComponent());
	ColliderComponent* cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eCAPSULE);
	go->AttachComponent(new Health());
	TestPlayer* player = go->AttachComponent(new TestPlayer());
	BodyInstanceData lock;
	lock.LockXRot = true;
	lock.LockZRot = true;
	go->GetComponent<RigidbodyComponent>()->SetBodyData(lock);

	GameObject* Cam = new GameObject("PlayerCamera");
	Cam->SetParent(go);

	player->CameraObject = Cam;


	WeaponManager* manager = go->AttachComponent(new WeaponManager());
	manager->Weapons[Weapon::WeaponType::Rifle] = go->AttachComponent(new Weapon(Weapon::WeaponType::Rifle, Scene, player));
	manager->Weapons[Weapon::WeaponType::ShotGun] = go->AttachComponent(new Weapon(Weapon::WeaponType::ShotGun, Scene, player));
	manager->Weapons[Weapon::WeaponType::RailGun] = go->AttachComponent(new Railgun(Scene, player));
	manager->Melee = go->AttachComponent(new MeleeWeapon());
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	cc->Radius = 10.0f;
	cc->IsTrigger = true;
	manager->Melee->Collider = cc;
	Scene->AddGameobjectToScene(Cam);
	Scene->AddGameobjectToScene(go);
	AISystem::Get()->GetDirector<TestGame_Director>()->SetPlayer(player->GetOwner());
	//	SpawnSKull(glm::vec3(20, 5, 0));
	SpawnSKull(glm::vec3(-15, 5, 0));

	GameObject* AiTest = Scene::CreateDebugSphere(nullptr);
	AiTest->AttachComponent(new SpawningPool());
	AiTest->SetPosition(glm::vec3(50, -8, 10));
	Scene->AddGameobjectToScene(AiTest);
	Pickup::SpawnPickup(glm::vec3(0, 1, -10), PickupType::Rifle_Ammo, 10);
#if 0
	GameObject* AiTest = MakeTestSphere(Scene);
	AiTest->SetPosition(glm::vec3(50, -2, 0));
	AIController* controller = AiTest->AttachComponent(new AIController());
	controller->MoveTo(player->GetOwner());
#endif
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
	skull->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\Munkey.obj"), mat));
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
	GameMode::EndPlay();
}

void TestGameGameMode::Update()
{
	GameMode::Update();
}
