#include "Scene.h"
#include "../RHI/RHI.h"
#include "../Components/MeshRendererComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/LightComponent.h"
#include "../Rendering/Renderers/RenderEngine.h"
#include "../Components/CompoenentRegistry.h"
#include "../Components/RigidbodyComponent.h"
Scene::Scene()
{}

Scene::~Scene()
{}

void Scene::UpdateScene(float deltatime)
{
	StaticSceneNeedsUpdate = false;//clear last frames flag
	if (SceneObjects.size() == 0)
	{
		return;
	}
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->Update(deltatime);
	}
}
void Scene::EditorUpdateScene()
{
	StaticSceneNeedsUpdate = false;//clear last frames flag
	if (SceneObjects.size() == 0)
	{
		return;
	}
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->EditorUpdate();
	}
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
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->BeginPlay();
	}
}
//load an example scene
void Scene::LoadDefault(RenderEngine* Renderer, bool IsDeferredMode)
{
	GameObject* go = new GameObject("House");

	Material* newmat = new Material(RHI::CreateTexture("house_diffuse.tga", true));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj", Renderer->GetMainShader()->GetShaderProgram()), newmat));
	go->GetTransform()->SetPos(glm::vec3(7, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	AddGameobjectToScene(go);

	go = new GameObject("Terrain");
	Material* mat = new Material(RHI::CreateTexture("grasshillalbedo.png"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("terrainmk2.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	//slow
	AddGameobjectToScene(go);

	go = new GameObject("Camera");
	go->GetTransform()->SetPos(glm::vec3(0, 10, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));
	go->AttachComponent(new CameraComponent());
	AddGameobjectToScene(go);

	go = new GameObject("LightTest");
	go->GetTransform()->SetPos(glm::vec3(0, 10, 20));
	LightComponent* lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->Internal_GetLightPtr()->SetShadowId(0);
	lc->SetIntensity(100);
	AddGameobjectToScene(go);

	go = new GameObject("Dir Light");
	go->GetTransform()->SetPos(glm::vec3(0, 5, 1));
	go->GetTransform()->SetEulerRot(glm::vec3(-90, 0, 0));
	/*LightComponent **/ lc = (LightComponent*)go->AttachComponent(new LightComponent());
//	lc->SetShadow(true);
	lc->SetLightType(Light::Directional);
	lc->SetIntensity(0.1f);
	AddGameobjectToScene(go);

	go = new GameObject("Plane");
	mat = new Material(RHI::CreateTexture("bricks2.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 20, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));
	go->AttachComponent(new RigidbodyComponent());
	AddGameobjectToScene(go);



	go = new GameObject("Plane");
	mat = new Material(RHI::CreateTexture("bricks2.jpg"));
	mat->NormalMap = RHI::CreateTexture("bricks2_normal.jpg");
	mat->DisplacementMap = RHI::CreateTexture("bricks2_disp.jpg");

	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
	go->GetTransform()->SetPos(glm::vec3(-24, 2, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));

	AddGameobjectToScene(go);

	go = new GameObject("Fence");
	mat = new Material(RHI::CreateTexture("fence.png"));
	mat->SetShadow(false);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
	go->GetTransform()->SetPos(glm::vec3(-10, 1, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(90, -90, 0));
	go->GetTransform()->SetScale(glm::vec3(0.1f));

	AddGameobjectToScene(go);



	go = new GameObject("Static Water");
	mat = new Material(RHI::CreateTexture("Water fallback.jpg"));
	mat->NormalMap = RHI::CreateTexture("IKT4l.jpg");
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
	go->GetTransform()->SetPos(glm::vec3(-37, -2, -20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));

	AddGameobjectToScene(go);
	if (IsDeferredMode == false)
	{
		go = new GameObject("Water");

		Material::MaterialProperties props;
		props.DoesShadow = false;
		props.IsReflective = true;
		mat = new Material(Renderer->GetReflectionBuffer()->GetRenderTexture(), props);
		mat->NormalMap = RHI::CreateTexture("IKT4l.jpg");
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
		go->GetTransform()->SetPos(glm::vec3(-37, -1, -21));
		go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
		go->GetTransform()->SetScale(glm::vec3(2));
		Camera* c = new Camera(go->GetTransform()->GetPos(), 90.0f, static_cast<float>(512 / 512), 0.1f, 100.0f);
		c->Pitch(-90);
		c->SetUpAndForward(glm::vec3(0, 1.0, 0), glm::vec3(0, 0, 1.0));
		Renderer->SetReflectionCamera(c);
		AddGameobjectToScene(go);
	}
	StaticSceneNeedsUpdate = true;
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
		//todo error!
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
void Scene::AddGameobjectToScene(GameObject* gameobject)
{
	SceneObjects.push_back(gameobject);
	gameobject->Internal_SetScene(this);
}

void Scene::CopyScene(Scene* newscene)
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		GameObject* go = new GameObject(*SceneObjects[i]);
		newscene->AddGameobjectToScene(go);
	}

}