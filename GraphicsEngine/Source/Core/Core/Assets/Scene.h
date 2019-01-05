#pragma once
#include "Core/GameObject.h"
#include "Rendering/Core/Light.h"
class RenderEngine;
class GameMode;
class Scene
{
public:
	Scene(bool EditScene = false);
	~Scene();
	void AlwaysUpdate(float deltatime);
	void UpdateScene(float deltatime);
#if WITH_EDITOR
	void EditorUpdateScene();
#endif
	void OnFrameEnd();
	void FixedUpdateScene(float deltatime);
	CORE_API void AddGameobjectToScene(GameObject* gameobject);
	void CopyScene(Scene * newscene);
	std::vector<GameObject*>* GetObjects()
	{
		return &SceneObjects;
	}
	std::vector<GameObject*>* GetMeshObjects()
	{
		return &RenderSceneObjects;
	}
	void StartScene();
	void LoadDefault();
	void LoadExampleScene(RenderEngine * Renderer, bool IsDeferredMode);

	void SpawnDoor(std::string name, glm::vec3 pos);

	std::vector<Light*>* GetLights()
	{
		return &Lights;
	}
	Camera* GetCurrentRenderCamera()
	{
		return CurrentCamera;
	}
	void AddCamera(Camera* cam)
	{
		Cameras.emplace_back(cam); CurrentCamera = cam;
	}//todo: Camera priority
	void RemoveCamera(Camera* Cam);
	void AddLight(Light* Light)
	{
		Lights.emplace_back(Light);
	}//todo: Camera priority
	void RemoveLight(Light* Light);
	void RemoveGameObject(GameObject * object);
	bool StaticSceneNeedsUpdate = false;
	void EndScene();
	struct LightingEnviromentData
	{
		BaseTexture* SkyBox = nullptr;
		BaseTexture* DiffuseMap = nullptr;
	};
	LightingEnviromentData* GetLightingData()
	{
		return &LightingData;
	}
	bool IsEditorScene()
	{
		return bEditorScene;
	}
	void TickDeferredRemove();
	CORE_API static GameObject* CreateDebugSphere(Scene* s);
	float GetGravityStrength() const
	{
		return GravityStrength;
	}
	GameMode* GetGameMode()
	{
		return CurrentGameMode;
	}
	bool IsSceneDestorying()const
	{
		return IsDestruction;
	}
	int FindAllOfName(std::string name, std::vector<GameObject*>& Objects);
	CORE_API GameObject * FindByName(std::string name);
private:
	bool IsDestruction = false;
	float GravityStrength = 20.0f;
	std::vector<GameObject*> SceneObjects;
	std::vector<GameObject*> RenderSceneObjects;
	std::vector<Light*> Lights;
	std::vector<Camera*> Cameras;
	Camera* CurrentCamera = nullptr;
	GameMode* CurrentGameMode = nullptr;
	bool bEditorScene = false;
	LightingEnviromentData LightingData = LightingEnviromentData();
	bool IsRunning = false;
	std::vector<GameObject*> DeferredRemoveQueue;
};

