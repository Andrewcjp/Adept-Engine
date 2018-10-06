#include "Scene.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Components/CameraComponent.h"
#include "Core/Components/LightComponent.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Core/Components/CompoenentRegistry.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Core/Components/Utillity/FreeLookComponent.h"
#include <algorithm>
#include "Core/Utils/MemoryUtils.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Assets/Asset_Shader.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Game/Gamemode.h"
#include "Core/Game/Game.h"
#include "Core/Components/ColliderComponent.h"
Scene::Scene(bool EditScene)
{
	LightingData.SkyBox = AssetManager::DirectLoadTextureAsset("\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds", true);
	LightingData.SkyBox->AddRef();
	CurrentGameMode = Engine::GetGame()->CreateGameMode();
	bEditorScene = EditScene;
}

Scene::~Scene()
{
	Lights.clear();
	MemoryUtils::DeleteVector(SceneObjects);
	SafeRHIRefRelease(LightingData.SkyBox);
	SafeRHIRefRelease(LightingData.DiffuseMap);
	delete CurrentGameMode;
}

void Scene::UpdateScene(float deltatime)
{
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
}

void Scene::StartScene()
{
	IsRunning = true;
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
	GameObject* go = new GameObject("House");
	LightComponent* lc = nullptr;
	Material* mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\house_diffuse.tga"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(7, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	AddGameobjectToScene(go);

	go = new GameObject("Terrain");
	mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\grasshillalbedo.png"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("DefaultTerrain.obj"), mat));
	go->AttachComponent(new LightComponent());
	go->GetTransform()->SetPos(glm::vec3(0, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	go->AttachComponent(new ColliderComponent());
	AddGameobjectToScene(go);

	go = new GameObject("Camera");
	go->GetTransform()->SetPos(glm::vec3(0, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	go->AttachComponent(new CameraComponent());
#if !WITH_EDITOR
	go->AttachComponent(new FreeLookComponent());
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
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Sphere.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(10, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	AddGameobjectToScene(go);
	Asset_Shader* ColourMat = new Asset_Shader();
	ColourMat->SetupSingleColour();

	go = new GameObject("Test");
	mat = ColourMat->GetMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->GetProperties()->Roughness = 0.0f;
	mat->GetProperties()->Metallic = 1.0f;

	//mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_normal.jpg", true));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Sphere.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 10, 10));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(1));
	go->AttachComponent(new ColliderComponent());
	go->AttachComponent(new RigidbodyComponent());
	AddGameobjectToScene(go);

	go = new GameObject("Plane");
	mat = NormalMapShader->GetMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_normal.jpg"));
	//	mat->SetDisplacementMap(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_disp.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(-24, 2, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));
	AddGameobjectToScene(go);

	go = new GameObject("Plane");
	mat = NormalMapShader->GetMaterialInstance();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2.jpg"));
	mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_normal.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 20, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));

	go->AttachComponent(new RigidbodyComponent());
	go->AttachComponent(new ColliderComponent());
	//go->actor = Engine::GetPhysEngineInstance()->CreatePrimitiveRigidBody(go->GetTransform()->GetPos(), glm::vec3(0, 10, 0), 1);
	AddGameobjectToScene(go);

	/*go = new GameObject("Fence");
	mat = new Material(AssetManager::DirectLoadTextureAsset("\\texture\\fence.png"));
	mat->SetShadow(false);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(-10, 1, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(90, -90, 0));
	go->GetTransform()->SetScale(glm::vec3(0.1f));

	AddGameobjectToScene(go);*/

	go = new GameObject("Static Water");
	mat = Material::GetDefaultMaterial();
	mat->SetDiffusetexture(AssetManager::DirectLoadTextureAsset("texture\\Water fallback.jpg"));
	//mat->SetNormalMap(AssetManager::DirectLoadTextureAsset("texture\\IKT4l.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(-37, -2, -20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	AddGameobjectToScene(go);


	/*go = new GameObject("Water");

	Material::MaterialProperties props;
	props.DoesShadow = false;
	props.IsReflective = true;
	mat = new Material(Renderer->GetReflectionBuffer()->GetRenderTexture(), props);
	mat->NormalMap = AssetManager::DirectLoadTextureAsset("\\texture\\IKT4l.jpg");
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
	go->GetTransform()->SetPos(glm::vec3(-37, -1, -21));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	Camera* c = new Camera(go->GetTransform()->GetPos(), 90.0f, static_cast<float>(512 / 512), 0.1f, 100.0f);
	c->Pitch(-90);
	c->SetUpAndForward(glm::vec3(0, 1.0, 0), glm::vec3(0, 0, 1.0));
	Renderer->SetReflectionCamera(c);
	AddGameobjectToScene(go);*/


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
	SafeRHIRefRelease(AssetManager::DirectLoadTextureAsset("\\texture\\bricks2_normal.jpg"));
	FrameBuffer* fv = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), RHIFrameBufferDesc::CreateColour(1, 1));
	EnqueueSafeRHIRelease(fv);
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
	SceneObjects.erase(std::remove(SceneObjects.begin(), SceneObjects.end(), object));
	object->Internal_SetScene(nullptr);
}
void Scene::EndScene()
{
	CurrentGameMode->EndPlay();
	IsRunning = false;
}

void Scene::AddGameobjectToScene(GameObject* gameobject)
{
	SceneObjects.push_back(gameobject);
	if (gameobject->GetMat() != nullptr)
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