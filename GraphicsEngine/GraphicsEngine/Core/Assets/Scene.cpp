#include "Scene.h"
#include "../RHI/RHI.h"
#include "../Components/MeshRendererComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/LightComponent.h"
#include "../Rendering/Renderers/RenderEngine.h"
#include "../Components/CompoenentRegistry.h"
#include "../Components/RigidbodyComponent.h"
#include <algorithm>
#include "../Core/Utils/MemoryUtils.h"
Scene::Scene()
{}

Scene::~Scene()
{
	Lights.clear();
	MemoryUtils::DeleteVector(SceneObjects);	
}

void Scene::UpdateScene(float deltatime)
{
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
	if (SceneObjects.size() == 0)
	{
		return;
	}
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->EditorUpdate();
	}
}

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
	GameObject* go = new GameObject("House");
	LightComponent* lc = nullptr;
	Material* newmat = new Material(RHI::CreateTexture("\\asset\\texture\\house_diffuse.tga"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("house.obj"), newmat));
	go->GetTransform()->SetPos(glm::vec3(7, 0, 0));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	AddGameobjectToScene(go);

	go = new GameObject("Terrain");
	Material* mat = new Material(RHI::CreateTexture("\\asset\\texture\\grasshillalbedo.DDS"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("terrainmk2.obj"), mat));
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

#if 0
	go = new GameObject("Dir Light");
	go->GetTransform()->SetPos(glm::vec3(0, 5, 1));
	go->GetTransform()->SetEulerRot(glm::vec3(45, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Directional);
	lc->SetIntensity(1.3f);
	AddGameobjectToScene(go);

	//go = new GameObject("Dir Light");
	//go->GetTransform()->SetPos(glm::vec3(0, 5, 1));
	//go->GetTransform()->SetEulerRot(glm::vec3(50, 0, 0));
	//lc = (LightComponent*)go->AttachComponent(new LightComponent());
	//lc->SetShadow(true);
	//lc->SetLightType(Light::Directional);
	//lc->SetIntensity(1.3f);
	//AddGameobjectToScene(go);
#endif

#if 1
	go = new GameObject("Point Light");
	go->GetTransform()->SetPos(glm::vec3(0, 5, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Point);
	lc->SetIntensity(1.0f);
	AddGameobjectToScene(go);

	go = new GameObject("Point Light");
	go->GetTransform()->SetPos(glm::vec3(10, 5, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	lc = (LightComponent*)go->AttachComponent(new LightComponent());
	lc->SetShadow(true);
	lc->SetLightType(Light::Point);
	lc->SetIntensity(2.0f);
	AddGameobjectToScene(go);
	/*for (int i = 0; i < 3; i++)
	{
		go = new GameObject("Point Light");
		go->GetTransform()->SetPos(glm::vec3(20 + (10 * i), 5, 20));
		go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
		lc = (LightComponent*)go->AttachComponent(new LightComponent());
		lc->SetShadow(true);
		lc->SetLightType(Light::Point);
		lc->SetIntensity(2.0f);
		AddGameobjectToScene(go);
	}*/
#endif

	go = new GameObject("Plane");
	mat = new Material(RHI::CreateTexture("\\asset\\texture\\bricks2.DDS"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(0, 20, 20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));
	go->AttachComponent(new RigidbodyComponent());

	AddGameobjectToScene(go);

	go = new GameObject("Plane");
	mat = new Material(RHI::CreateTexture("\\asset\\texture\\bricks2.DDS"));
	mat->SetNormalMap(RHI::CreateTexture("\\asset\\texture\\bricks2_normal.jpg"));
	mat->SetDisplacementMap(RHI::CreateTexture("\\asset\\texture\\bricks2_disp.jpg"));

	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(-24, 2, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(0.5));

	AddGameobjectToScene(go);

	/*go = new GameObject("Fence");
	mat = new Material(RHI::CreateTexture("\\asset\\texture\\fence.png"));
	mat->SetShadow(false);
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(-10, 1, -6));
	go->GetTransform()->SetEulerRot(glm::vec3(90, -90, 0));
	go->GetTransform()->SetScale(glm::vec3(0.1f));

	AddGameobjectToScene(go);*/

	go = new GameObject("Static Water");
	mat = new Material(RHI::CreateTexture("\\asset\\texture\\Water fallback.jpg"));
	mat->SetNormalMap(RHI::CreateTexture("\\asset\\texture\\IKT4l.jpg"));
	go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj"), mat));
	go->GetTransform()->SetPos(glm::vec3(-37, -2, -20));
	go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
	go->GetTransform()->SetScale(glm::vec3(2));

	AddGameobjectToScene(go);
	if (IsDeferredMode == false)
	{
		/*go = new GameObject("Water");

		Material::MaterialProperties props;
		props.DoesShadow = false;
		props.IsReflective = true;
		mat = new Material(Renderer->GetReflectionBuffer()->GetRenderTexture(), props);
		mat->NormalMap = RHI::CreateTexture("\\asset\\texture\\IKT4l.jpg");
		go->AttachComponent(new MeshRendererComponent(RHI::CreateMesh("Plane.obj", Renderer->GetMainShader()->GetShaderProgram()), mat));
		go->GetTransform()->SetPos(glm::vec3(-37, -1, -21));
		go->GetTransform()->SetEulerRot(glm::vec3(0, 0, 0));
		go->GetTransform()->SetScale(glm::vec3(2));
		Camera* c = new Camera(go->GetTransform()->GetPos(), 90.0f, static_cast<float>(512 / 512), 0.1f, 100.0f);
		c->Pitch(-90);
		c->SetUpAndForward(glm::vec3(0, 1.0, 0), glm::vec3(0, 0, 1.0));
		Renderer->SetReflectionCamera(c);
		AddGameobjectToScene(go);*/
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