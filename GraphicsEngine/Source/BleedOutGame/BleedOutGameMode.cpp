#include "BleedOutGameMode.h"
#include "AI/Core/AIController.h"
#include "AI/Core/AISystem.h"
#include "AI/SkullChaser.h"
#include "AI/BleedOut_Director.h"
#include "Components/Health.h"
#include "Components/MeleeWeapon.h"
#include "Components/Pickup.h"
#include "Components/Projectile.h"
#include "Components/Railgun.h"
#include "Components/SpawningPool.h"
#include "Components/BleedOutPlayer.h"
#include "Components/Weapon.h"
#include "Components/WeaponManager.h"
#include "Core/Components/Core_Components_inc.h"
#include "Physics/GenericConstraint.h"
#include "Audio/AudioEngine.h"
#include "BleedOutHud.h"

BleedOutGameMode::BleedOutGameMode()
{}

BleedOutGameMode::~BleedOutGameMode()
{}

GameObject* MakeTestSphere(Scene* Scene, bool Static = false)
{
	GameObject* go = Scene::CreateDebugSphere(nullptr);
	if (!Static)
	{
		go->AttachComponent(new RigidbodyComponent());
	}
	else
	{
		go->SetPosition(glm::vec3(10, 20, 0));
	}
	ColliderComponent* cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	Scene->AddGameobjectToScene(go);
	return go;
}

void BleedOutGameMode::BeginPlay(Scene* Scene)
{
	GameMode::BeginPlay(Scene);
#if 1
	GameObject* A = MakeTestSphere(Scene, true);
	GameObject* B = MakeTestSphere(Scene);
	B->SetPosition(glm::vec3(10, 10, 0));
	ConstaintSetup data;
	data.Desc.Dampening = 0.2f;
	data.Desc.SpringK = 1.0f;
	ConstraintInstance* aint = Engine::GetPhysEngineInstance()->CreateConstraint(A->GetComponent<ColliderComponent>()->GetActor(), B->GetComponent<RigidbodyComponent>()->GetActor(), data);
#endif

	SpawnPlayer(glm::vec3(0, 10, 10), Scene);

	GameObject* AiTest = new GameObject();
	AiTest->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Models\\SpawningPool.obj"), Material::GetDefaultMaterial()));
	AiTest->AttachComponent(new SpawningPool());
	AiTest->SetPosition(glm::vec3(0, 0, 10));
	Scene->AddGameobjectToScene(AiTest);
	Pickup::SpawnPickup(glm::vec3(0, 1, -10), PickupType::Rifle_Ammo, 10);
	Pickup::SpawnPickup(glm::vec3(0, 1, -12), PickupType::Health, 10);
#if 0
	GameObject* AiTest = MakeTestSphere(Scene);
	AiTest->SetPosition(glm::vec3(50, -2, 0));
	AIController* controller = AiTest->AttachComponent(new AIController());
	controller->MoveTo(player->GetOwner());
#endif
}

void BleedOutGameMode::SpawnSKull(glm::vec3 Position)
{
	GameObject* skull = GameObject::Instantiate(Position);
	SkullChaser* c = skull->AttachComponent(new SkullChaser());
	c->Player = MPlayer;
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

void BleedOutGameMode::EndPlay()
{
	GameMode::EndPlay();
}

void BleedOutGameMode::Update()
{
	GameMode::Update();
}

void BleedOutGameMode::OnPlayerDeath()
{
	GetPlayer()->Destory();
	BleedOutHud* Tghud = (BleedOutHud*)Hud;
	Tghud->ShowRestart();

}

void BleedOutGameMode::SpawnPlayer(glm::vec3 Pos, Scene* Scene)
{
	GameObject* go = new GameObject("Player Test");
	MPlayer = go;
	MPlayer->Tags.Add("player");
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\Sphere.obj"), mat));
	go->SetPosition(Pos);
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	go->AttachComponent(new RigidbodyComponent());
	ColliderComponent* cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eCAPSULE);
	Health* H = go->AttachComponent(new Health());
	H->BindDeathCallback(std::bind(&BleedOutGameMode::OnPlayerDeath, this));
	BleedOutPlayer* player = go->AttachComponent(new BleedOutPlayer());
	BodyInstanceData lock;
	lock.LockXRot = true;
	lock.LockZRot = true;
	lock.Mat = new PhysicalMaterial();
	lock.Mat->Bouncyness = 0.0f;
	lock.Mat->StaticFriction = 0.0f;
	go->GetComponent<RigidbodyComponent>()->SetBodyData(lock);

	GameObject* Cam = new GameObject("PlayerCamera");
	AudioEngine::Get()->MakeDefaultListener(Cam);
	Cam->SetParent(go);
	player->CameraObject = Cam;
	WeaponManager* manager = go->AttachComponent(new WeaponManager());
	manager->Weapons[Weapon::WeaponType::Rifle] = go->AttachComponent(new Weapon(Weapon::WeaponType::Rifle, Scene, player));
	manager->Weapons[Weapon::WeaponType::ShotGun] = go->AttachComponent(new Weapon(Weapon::WeaponType::ShotGun, Scene, player));
	manager->Weapons[Weapon::WeaponType::RailGun] = go->AttachComponent(new Railgun(Scene, player));
	manager->Melee = go->AttachComponent(new MeleeWeapon());
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	cc->Radius = 2.0f;
	cc->IsTrigger = true;
	manager->Melee->Collider = cc;


	Scene->AddGameobjectToScene(Cam);
	Scene->AddGameobjectToScene(go);
	AISystem::Get()->GetDirector<BleedOut_Director>()->SetPlayer(player->GetOwner());
}
