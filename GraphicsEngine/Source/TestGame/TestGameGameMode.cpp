#include "TestGameGameMode.h"
#include "Core/Components/Core_Components_inc.h"
#include "Components/TestPlayer.h"
#include "Components/Weapon.h"
#include "AI/SkullChaser.h"
#include "Components/Health.h"
#include "Components/Projectile.h"
#include "Components/WeaponManager.h"
#include "AI/Core/AIController.h"
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
#if 1//TDSIM_ENABLED
	//return; 
#endif
#if 1
	GameObject* go = new GameObject("Player Test");
	player = go;
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

	TestPlayer* player = go->AttachComponent(new TestPlayer());
	BodyInstanceData lock;
	lock.LockXRot = true;
	lock.LockZRot = true;
	go->GetComponent<RigidbodyComponent>()->SetLockFlags(lock);
	
	GameObject* Cam = new GameObject("PlayerCamera");
	Cam->SetParent(go);
	
	player->CameraObject = Cam;
	Scene->AddGameobjectToScene(Cam);
	Scene->AddGameobjectToScene(go);

	WeaponManager* manager = go->AttachComponent(new WeaponManager());
	manager->Weapons[0] = go->AttachComponent(new Weapon());
	manager->Weapons[1] = go->AttachComponent(new Weapon());
	manager->Weapons[2] = go->AttachComponent(new Weapon());	
	go = new GameObject("Gun Test");
	mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Weapons\\Rifle\\Textures\\Variation 06\\Rifle_06_Albedo.png"));
	MeshLoader::FMeshLoadingSettings set;
	set.FlipUVs = true;
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Weapons\\Rifle\\Rifle.fbx", set), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 2, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	Scene->AddGameobjectToScene(go);
	manager->Weapons[0]->SetWeaponModel(go, player->CameraObject);
	SpawnSKull(glm::vec3(20, 5, 0));
	SpawnSKull(glm::vec3(-15, 5, 0));
#endif

	GameObject* AiTest = MakeTestSphere(Scene);
	AiTest->SetPosition(glm::vec3(10, 10, 0));
	AIController* controller = AiTest->AttachComponent(new AIController());
	controller->MoveTo(glm::vec3(0, 0, 20));
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

}

void TestGameGameMode::Update()
{

}
