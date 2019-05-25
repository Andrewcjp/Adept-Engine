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
	TextureImportSettings s;
	s.IsCubeMap = true;
	LightingData.SkyBox = AssetManager::DirectLoadTextureAsset("\\texture\\MarsSky.dds", s);
	CurrentGameMode = Engine::GetGame()->CreateGameMode();
	bEditorScene = EditorScene;
}

Scene::~Scene()
{
	IsDestruction = true;
	Lights.clear();//Scene Does not own these objects
	MemoryUtils::DeleteVector(SceneObjects);
	//	SafeRHIRefRelease(LightingData.SkyBox);
	//	SafeRHIRefRelease(LightingData.DiffuseMap);
	SafeDelete(CurrentGameMode);
}
void Scene::AlwaysUpdate(float deltatime)
{
	CurrentGameMode->AlwaysUpdate();
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

void Scene::AddLight(glm::vec3 Pos, bool Shadow, float BrightNess)
{
	GameObject *go = new GameObject("Point Light");
	go->GetTransform()->SetPos(Pos);
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	LightComponent* lc = go->AttachComponent(new LightComponent());
	lc->SetShadow(Shadow);
	lc->SetLightType(Light::Point);
	lc->SetIntensity(BrightNess);
	AddGameobjectToScene(go);
}

GameObject* Scene::SpawnBox(glm::vec3 pos)
{
	GameObject *go = new GameObject("Rock");
	Material *mat = Material::CreateDefaultMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Props\\Crate_2\\Crate_Diffuse.png"));
	MeshLoader::FMeshLoadingSettings set = MeshLoader::FMeshLoadingSettings();
	set.FlipUVs = true;
	set.Scale = glm::vec3(.01f);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Crate_Cube.fbx", set), mat));
	go->GetTransform()->SetPos(pos);
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	ColliderComponent*	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eBOX);
	cc->LocalOffset = glm::vec3(0, 1, 0);
	go->AttachComponent(new RigidbodyComponent());
	AddGameobjectToScene(go);
	return go;
}
//load an example scene
void Scene::LoadExampleScene(RenderEngine* Renderer, bool IsDeferredMode)
{
	StaticSceneNeedsUpdate = true;
	Asset_Shader* NormalMapShader = new Asset_Shader();
	NormalMapShader->SetupTestMat();
	GameObject* go = nullptr;
	Material* mat = nullptr;
	ColliderComponent* cc = nullptr;
#if 0
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
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eTRIANGLEMESH);
	cc->SetTriangleMeshAssetName(Name);

	AddGameobjectToScene(go);
#else
	go = new GameObject("Terrain");
	mat = Material::CreateDefaultMaterialInstance();
	MeshLoader::FMeshLoadingSettings set;
#if 0
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\Terrain\\MeduimMap.png"));
	set.UVScale = glm::vec2(1);
#else
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\Terrain\\textures_industrial_floors_floor_paint_lightgray_c.png"));
	set.UVScale = glm::vec2(20);
#endif
	set.FlipUVs = true;
	const char* Name = "\\AlwaysCook\\Terrain\\Room1.obj";
	MeshRendererComponent* r = go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh(Name, set), mat));//TerrrainTest
	mat = Material::CreateDefaultMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\Terrain\\textures_industrial_floors_floor_paint_lightgray_c.png"));
	r->SetMaterial(mat, 1);
	mat = Material::CreateDefaultMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\BoxObject.png"));
	r->SetMaterial(mat, 2);
	go->GetTransform()->SetPos(glm::vec3(0, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eTRIANGLEMESH);
	cc->SetTriangleMeshAssetName(Name);
	AddGameobjectToScene(go);

#endif
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
	//sun
	AddLight(glm::vec3(-8, 27, -12), true, 2500.0f);
	bool ExtraShadows = true;
	AddLight(glm::vec3(0, 10, -20), ExtraShadows, 200.0f);
	AddLight(glm::vec3(0, 5, 34), ExtraShadows, 75.0f);
	AddLight(glm::vec3(0, 4, -50), ExtraShadows, 75.0f);
	//light testing
#if 1
	for (int i = 0; i < RHI::GetRenderConstants()->MAX_LIGHTS - 4; i += 4)
	{
		AddLight(glm::vec3(24, 7, -21), false, 75.0f);
		AddLight(glm::vec3(33, 6, -3), false, 75.0f);

		AddLight(glm::vec3(-24, 7, -21), false, 75.0f);
		AddLight(glm::vec3(-33, 6, -3), false, 75.0f);
	}
#endif
	go = new GameObject("Rock");
	mat = Material::CreateDefaultMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Props\\Crate_1\\low_default_AlbedoTransparency.png"/*, setting*/));//Crate_Diffuse
	set = MeshLoader::FMeshLoadingSettings();
	set.FlipUVs = true;
	set.Scale = glm::vec3(.01f);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Box_low.fbx", set), mat));
	go->GetTransform()->SetPos(glm::vec3(30, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eBOX);
	go->AttachComponent(new RigidbodyComponent());
	AddGameobjectToScene(go);

	Asset_Shader* ColourMat = new Asset_Shader();
	ColourMat->SetupSingleColour();



	go = new GameObject("spawn");
	go->GetTransform()->SetPos(glm::vec3(30, 5, -2));
	go->AttachComponent(new SpawnMarker());
	AddGameobjectToScene(go);

	go = new GameObject("spawn");
	go->GetTransform()->SetPos(glm::vec3(-30, 5, -5));
	go->AttachComponent(new SpawnMarker());
	AddGameobjectToScene(go);

	go = new GameObject("spawn");
	go->GetTransform()->SetPos(glm::vec3(6, 0, -5));
	go->AttachComponent(new SpawnMarker());
	AddGameobjectToScene(go);
#if 1
	//go = CreateDebugSphere(nullptr);
	//cc = go->AttachComponent(new ColliderComponent());
	//go->GetTransform()->SetPos(glm::vec3(0, 15, 10));
	//cc->SetCollisonShape(EShapeType::eSPHERE);
	//go->AttachComponent(new RigidbodyComponent());
	//AddGameobjectToScene(go);
	glm::vec3 startpos = glm::vec3(5, -5, 0);
	float stride = 5.0f;
	int size = 1;
	int zsize = 1;
	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			for (int z = 0; z < zsize; z++)
			{
				if (x == 0)
				{
					go = new GameObject("Test box");
					mat = Material::CreateDefaultMaterialInstance();
					mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("Props\\Crate_1\\low_default_AlbedoTransparency.png"));
					set = MeshLoader::FMeshLoadingSettings();
					set.FlipUVs = true;
					set.Scale = glm::vec3(.01f);
					go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Box_low.fbx", set), mat));
					cc = go->AttachComponent(new ColliderComponent());
					cc->SetCollisonShape(EShapeType::eBOX);
				}
				else
				{
					go = CreateDebugSphere(nullptr);
					cc = go->AttachComponent(new ColliderComponent());
					cc->SetCollisonShape(EShapeType::eSPHERE);
				}
				go->AttachComponent(new RigidbodyComponent());
				go->GetTransform()->SetPos(startpos + glm::vec3(x *stride, 15 + z * stride, y*stride));
				AddGameobjectToScene(go);
			}
		}
	}
#endif

#if 1
	size = 3;

	glm::vec3 startPos = glm::vec3(0, 5, 0);
	stride = 5.0f;
	CreateGrid(size, startPos, 5.0f);


#endif
	//CreateGrid(10, glm::vec3(0, 10, -20), 0.5f);
	SpawnBox(glm::vec3(17, 1, -12));
	SpawnBox(glm::vec3(17, 1, -9));
	SpawnBox(glm::vec3(14, 1, -12));

	SpawnDoor("EntryDoor", glm::vec3(0, 0, 25));
	SpawnDoor("ExitDoor", glm::vec3(0, 0, -34));

#if 1
	go = new GameObject("Water");
	mat = Material::CreateDefaultMaterialInstance();
	mat->MaterialCData.MaterialRenderType = EMaterialRenderType::Transparent;
	mat->UpdateShaderData();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	MeshLoader::FMeshLoadingSettings s;
	//s.AllowInstancing = false;
	MeshRendererComponent* c = go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("\\models\\RenderPlane.obj", s), mat));
	c->SetMaterial(mat, 0);
	go->GetTransform()->SetPos(glm::vec3(0, 12, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	AddGameobjectToScene(go);
#endif
	Log::LogMessage("Gird size " + std::to_string(size*size*size) + " GO count " + std::to_string(GetMeshObjects()->size()));
}

void Scene::CreateGrid(int size, glm::vec3 startPos, float stride)
{
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			for (int z = 0; z < size; z++)
			{
				GameObject* go = new GameObject("Water");
				go->SetMoblity(GameObject::Dynamic);
				Material* mat = Material::CreateDefaultMaterialInstance();
				mat->SetFloat("Roughness", x * (1.0f / (size - 1)));
				mat->SetFloat("Metallic", y * (1.0f / (size - 1)));
				mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
				MeshLoader::FMeshLoadingSettings s;
				//s.AllowInstancing = false;
				MeshRendererComponent* c = go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("\\models\\Sphere.obj", s), mat));
				c->SetMaterial(mat, 0);
				go->GetTransform()->SetPos(startPos + glm::vec3(x*stride, y*stride, z*stride));
				go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
				go->GetTransform()->SetScale(glm::vec3(1));
				AddGameobjectToScene(go);
			}
		}
	}
}

void Scene::SpawnDoor(std::string name, glm::vec3 pos)
{
	GameObject* go = new GameObject(name);
	Material* mat = Material::CreateDefaultMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\Terrain\\DoorTex.png"));
	MeshLoader::FMeshLoadingSettings set;
	set.FlipUVs = true;
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("AlwaysCook\\Terrain\\Door.obj", set), mat));
	go->GetTransform()->SetPos(pos);
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1.1f));
	ColliderComponent* cc = go->AttachComponent(new ColliderComponent());
	cc->SetCollisonShape(EShapeType::eBOX);
	cc->LocalOffset = glm::vec3(0, 2, 0);
	cc->BoxExtents = glm::vec3(3, 3, 1);
	AddGameobjectToScene(go);
}

void Scene::RemoveCamera(Camera * Cam)
{
	if (Cameras.size() > 1)
	{
		Cameras.erase(std::remove(Cameras.begin(), Cameras.end(), Cam));
	}
	else
	{
		Cameras.clear();
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
		Lights.clear();
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
	Material* mat = Material::CreateDefaultMaterialInstance();
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

int Scene::FindAllOfName(std::string name, std::vector<GameObject*>& Objects)
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		if (SceneObjects[i]->GetName() == name)
		{
			Objects.push_back(SceneObjects[i]);
		}
	}
	return (int)Objects.size();
}

GameObject* Scene::FindByName(std::string name)
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		if (SceneObjects[i]->GetName() == name)
		{
			return SceneObjects[i];
		}
	}
	return nullptr;
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