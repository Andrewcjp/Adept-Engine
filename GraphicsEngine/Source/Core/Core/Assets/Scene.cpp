#include "Scene.h"
#include "AI/Core/AISystem.h"
#include "Asset_Shader.h"
#include "AssetManager.h"
#include "Core/Components/CameraComponent.h"
#include "Core/Components/ColliderComponent.h"
#include "Core/Components/LightComponent.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Core/Game/Game.h"
#include "AI/Core/SpawnMarker.h"

Scene::Scene(bool EditorScene)
{
	//LightingData.SkyBox = AssetManager::DirectLoadTextureAsset("\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds", true);
	LightingData.SkyBox = AssetManager::DirectLoadTextureAsset("\\texture\\MarsSky.dds");

	LightingData.SkyBox->AddRef();
	CurrentGameMode = Engine::GetGame()->CreateGameMode();
	bEditorScene = EditorScene;
}

Scene::~Scene()
{
	IsDestruction = true;
	Lights.clear();//Scene Does not own these objects
	MemoryUtils::DeleteVector(SceneObjects);
	SafeRHIRefRelease(LightingData.SkyBox);
	SafeRHIRefRelease(LightingData.DiffuseMap);
	SafeDelete(CurrentGameMode);
}

void Scene::UpdateScene(float deltatime)
{
	AISystem::Get()->Tick(deltatime);
	CurrentGameMode->Update();
	if (SceneObjects.size() == 0)
	{
		return;
	}
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->Update(deltatime);
	}

}
#if WITH_EDITOR
void Scene::EditorUpdateScene()
{
	if (SceneObjects.size() == 0)
	{
		return;
	}
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->EditorUpdate();
	}
}
#endif
void Scene::OnFrameEnd()
{
	StaticSceneNeedsUpdate = false;//clear last frames flag
}

void Scene::FixedUpdateScene(float deltatime)
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->FixedUpdate(deltatime);
	}
	TickDeferredRemove();
}

void Scene::StartScene()
{
	IsRunning = true;
	AISystem::Get()->SetupForScene(this);
	CurrentGameMode->BeginPlay(this);
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->BeginPlay();
	}
}

void Scene::LoadDefault()
{
	GameObject* go = new GameObject("Main Camera");
	go->GetTransform()->SetPos(glm::vec3(0, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	go->AttachComponent(new CameraComponent());
	AddGameobjectToScene(go);
}
//load an example scene
void Scene::LoadExampleScene(RenderEngine* Renderer, bool IsDeferredMode)
{
	StaticSceneNeedsUpdate = true;
	Asset_Shader* NormalMapShader = new Asset_Shader();
	NormalMapShader->SetupTestMat();
	GameObject* go = nullptr;
	LightComponent* lc = nullptr;
	Material* mat = nullptr;
	ColliderComponent* cc = nullptr;

	go = new GameObject("Terrain");
	mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\textures_terrain_ground_marsrock_ground_01_tiled_c.dds"));
	MeshLoader::FMeshLoadingSettings set;
	set.UVScale = glm::vec2(20);
	const char* Name = "models\\Room1.obj";
	MeshRendererComponent* r = go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh(Name, set), mat));//TerrrainTest
	mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	r->SetMaterial(mat, 1);
	go->AttachComponent(new LightComponent());
	go->GetTransform()->SetPos(glm::vec3(0, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
#if 1
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eTRIANGLEMESH);
	cc->SetTriangleMeshAssetName(Name);
#endif
	AddGameobjectToScene(go);

	go = new GameObject("Camera");
	go->GetTransform()->SetPos(glm::vec3(0, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	go->GetTransform()->GetScale();
	go->AttachComponent(new CameraComponent());
#if !WITH_EDITOR
	//go->AttachComponent(new FreeLookComponent());
#endif
	AddGameobjectToScene(go);

#if 0
	go = new GameObject("Dir Light");
	go->GetTransform()->SetPos(glm::vec3(0, 5, 1));
	go->GetTransform()->SetEulerRot(glm::vec3(45, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Directional);
	lc->SetIntensity(1.3f);
	AddGameobjectToScene(go);

#endif

#if 1
	go = new GameObject("Point Light");
	go->GetTransform()->SetPos(glm::vec3(38, 10, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Point);
	lc->SetIntensity(500.0f);
	AddGameobjectToScene(go);

	go = new GameObject("Point Light");
	go->GetTransform()->SetPos(glm::vec3(10, 20, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Point);
	lc->SetIntensity(500.0f);
	AddGameobjectToScene(go);

	go = new GameObject("Point Light");
	go->GetTransform()->SetPos(glm::vec3(3, 15, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Point);
	lc->SetIntensity(500.0f);
	AddGameobjectToScene(go);

	for (int i = 0; i < 1; i++)
	{
		go = new GameObject("Point Light");
		go->GetTransform()->SetPos(glm::vec3(20 + (10 * i), 5, 20));
		go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
		lc = (LightComponent*)go->AttachComponent(new LightComponent());
		lc->SetShadow(true);
		lc->SetLightType(Light::Point);
		lc->SetIntensity(500.0f);
		AddGameobjectToScene(go);
	}
#endif	

	go = new GameObject("Test");
	mat = NormalMapShader->GetMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_normal.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\Sphere.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(10, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	//ColliderComponent* col = go->AttachComponent(new ColliderComponent());
	//col->SetCollisonShape(EShapeType::eSPHERE);
	//col->Radius = 1.0f;
	AddGameobjectToScene(go);

	go = new GameObject("Rock");
	//mat = NormalMapShader->GetMaterialInstance();
	mat = Material::GetDefaultMaterial();
	mat->GetProperties()->Metallic = 0.0f;
	mat->GetProperties()->Roughness = 1.0f;
	TextureImportSettings setting;
	setting.Compression = ECompressionSetting::BRGA;
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Terrain\\Rock1.tif", setting));//Rock1_nmp
	//setting.Compression = ECompressionSetting::None;
	//mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("Terrain\\Rock1_nmp.tif", setting));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Terrain\\Rock1A.fbx"), mat));
	go->GetTransform()->SetPos(glm::vec3(10, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	AddGameobjectToScene(go);



	go = new GameObject("Rock");
	//mat = NormalMapShader->GetMaterialInstance();
	mat = Material::GetDefaultMaterial();
	//mat->GetProperties()->Metallic = 0.0f;
	//mat->GetProperties()->Roughness = 1.0f;

	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Props\\Crate_2\\Crate_Diffuse.png"/*, setting*/));//Crate_Diffuse
	set = MeshLoader::FMeshLoadingSettings();
	set.FlipUVs = true;
	set.Scale = glm::vec3(.01f);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Props\\Crate_2\\Crate_Cube.fbx",set), mat));
	go->GetTransform()->SetPos(glm::vec3(20, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	AddGameobjectToScene(go);

	go = new GameObject("Rock");
	//mat = NormalMapShader->GetMaterialInstance();
	mat = Material::GetDefaultMaterial();
	//mat->GetProperties()->Metallic = 0.0f;
	//mat->GetProperties()->Roughness = 1.0f;

	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Props\\Crate_1\\low_default_AlbedoTransparency.png"/*, setting*/));//Crate_Diffuse
	set = MeshLoader::FMeshLoadingSettings();
	set.FlipUVs = true;
	set.Scale = glm::vec3(.01f);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Props\\Crate_1\\Box_low.fbx", set), mat));
	go->GetTransform()->SetPos(glm::vec3(30, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	AddGameobjectToScene(go);

	//Military crate.fbx

	Asset_Shader* ColourMat = new Asset_Shader();
	ColourMat->SetupSingleColour(); 
#if 0
	go = new GameObject("Size Guide");
	mat = ColourMat->GetMaterialInstance();
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\sizeguide.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(50, -10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	AddGameobjectToScene(go);
#endif
	go = new GameObject("Test");
	mat = ColourMat->GetMaterialInstance();
	//mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;

	//mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_normal.jpg", true));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\Sphere.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(70, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
#if 0
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eSPHERE);
	go->AttachComponent(new RigidbodyComponent());
#endif
	AddGameobjectToScene(go);

	go = new GameObject("spawn");
	go->GetTransform()->SetPos(glm::vec3(50, 0, -5));
	go->AttachComponent(new SpawnMarker());
	AddGameobjectToScene(go);

	go = new GameObject("spawn");
	go->GetTransform()->SetPos(glm::vec3(50, 0, 10));
	go->AttachComponent(new SpawnMarker());
	AddGameobjectToScene(go);

	go = new GameObject("spawn");
	go->GetTransform()->SetPos(glm::vec3(60, 0, -5));
	go->AttachComponent(new SpawnMarker());
	AddGameobjectToScene(go);
#if 1
	//go = CreateDebugSphere(nullptr);
	//cc = go->AttachComponent(new ColliderComponent());
	//go->GetTransform()->SetPos(glm::vec3(0, 15, 10));
	//cc->SetCollisonShape(EShapeType::eSPHERE);
	//go->AttachComponent(new RigidbodyComponent());
	//AddGameobjectToScene(go);
	glm::vec3 startpos = glm::vec3(5, 0, 0); 
	float stride = 5.0f;
	int size = 5;
	int zsize = 2;
	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			for (int z = 0; z < zsize; z++)
			{
				go = CreateDebugSphere(nullptr);
				cc = go->AttachComponent(new ColliderComponent());
				go->GetTransform()->SetPos(startpos + glm::vec3(x *stride, 15 + z * stride, y*stride));
				if (z == 0)
				{
					cc->SetCollisonShape(EShapeType::eSPHERE);
				}
				else
				{
					cc->SetCollisonShape(EShapeType::eSPHERE);
				}
				go->AttachComponent(new RigidbodyComponent());
				AddGameobjectToScene(go);
			}
		}
}
#endif


#if 0
	int size = 5;
	glm::vec3 startPos = glm::vec3(10, 5, 30);
	float stride = 5.0f;
	Material::MaterialProperties props;
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			go = new GameObject("Water");
			mat = Material::GetDefaultMaterial();
			mat->GetProperties()->Roughness = x * (1.0f / (size - 1));
			mat->GetProperties()->Metallic = y * (1.0f / (size - 1));
			mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
			go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Sphere.obj"), mat));
			go->GetTransform()->SetPos(startPos + glm::vec3(x*stride, y*stride, 0));
			go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
			go->GetTransform()->SetScale(glm::vec3(1));
			AddGameobjectToScene(go);
		}
		}
#endif
	}

void Scene::RemoveCamera(Camera * Cam)
{
	if (Cameras.size() > 1)
	{
		Cameras.erase(std::remove(Cameras.begin(), Cameras.end(), Cam));
	}
	else
	{
		Cameras.empty();
	}
	if (CurrentCamera == Cam)
	{
		if (Cameras.size())
		{
			CurrentCamera = Cameras[0];
		}
		else
		{
			CurrentCamera = nullptr;
		}
	}
}

void Scene::RemoveLight(Light * Light)
{
	if (Lights.size() > 1)
	{
		Lights.erase(std::remove(Lights.begin(), Lights.end(), Light));
	}
	else
	{
		Lights.empty();
	}
}

void Scene::RemoveGameObject(GameObject* object)
{
	object->OnRemoveFromScene();
	DeferredRemoveQueue.push_back(object);
}

void Scene::EndScene()
{
	CurrentGameMode->EndPlay();
	AISystem::Get()->SceneEnd();
	IsRunning = false;
}

void Scene::TickDeferredRemove()
{
	for (int i = 0; i < DeferredRemoveQueue.size(); i++)
	{
		if (DeferredRemoveQueue[i] != nullptr)
		{
			auto vecso = std::find(SceneObjects.begin(), SceneObjects.end(), DeferredRemoveQueue[i]);
			if (vecso != std::end(SceneObjects))
			{
				SceneObjects.erase(vecso);
			}
			auto vec = std::find(RenderSceneObjects.begin(), RenderSceneObjects.end(), DeferredRemoveQueue[i]);
			if (vec != std::end(RenderSceneObjects))
			{
				RenderSceneObjects.erase(vec);
			}
			DeferredRemoveQueue[i]->Internal_SetScene(nullptr);
			SafeDelete(DeferredRemoveQueue[i]);
		}
	}
	DeferredRemoveQueue.clear();
}

GameObject * Scene::CreateDebugSphere(Scene* s)
{
	GameObject* go = new GameObject("Test Sphere");
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("models\\Sphere.obj"), mat));
	go->SetPosition(glm::vec3(0, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	if (s != nullptr)
	{
		s->AddGameobjectToScene(go);
	}
	return go;
}

void Scene::AddGameobjectToScene(GameObject* gameobject)
{
	SceneObjects.push_back(gameobject);
	if (gameobject->GetMesh() != nullptr)
	{
		RenderSceneObjects.push_back(gameobject);
	}
	gameobject->Internal_SetScene(this);
	if (IsRunning)
	{
		gameobject->BeginPlay();
	}
}

void Scene::CopyScene(Scene* newscene)
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		GameObject* go = new GameObject(*SceneObjects[i]);
		newscene->AddGameobjectToScene(go);
	}

}