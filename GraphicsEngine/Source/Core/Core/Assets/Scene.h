#pragma once
#include "Core/GameObject.h"
#include "Rendering/Core/Light.h"
#include "RHI/BaseTexture.h"
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
	std::vector<GameObject*>& GetObjects();
	std::vector<GameObject*>& GetMeshObjects();
	void StartScene();
	void LoadDefault();
	void AddLight(glm::vec3 Pos, bool Shadow, float BrightNess, float range = 10);
	CORE_API GameObject* SpawnBox(glm::vec3 pos);
	GameObject * AddMeshObject(glm::vec3 pos, std::string mesh, Material *mat = nullptr);
	void LoadExampleScene();

	void PadUntil(int target);

	void CreateGrid(int size, glm::vec3 startPos, float stride, bool OneD = false);
	void SpawnDoor(std::string name, glm::vec3 pos);

	std::vector<Light*>& GetLights();
	Camera* GetCurrentRenderCamera();
	void AddCamera(Camera* cam);//#Scene Camera priority
	void RemoveCamera(Camera* Cam);
	void AddLight(Light* Light);
	void RemoveLight(Light* Light);
	void RemoveGameObject(GameObject * object);
	bool StaticSceneNeedsUpdate = false;
	void EndScene();
	struct LightingEnviromentData
	{
		BaseTextureRef SkyBox = nullptr;
		BaseTextureRef DiffuseMap = nullptr;
	};
	LightingEnviromentData* GetLightingData();
	bool IsEditorScene();
	void TickDeferredRemove();
	CORE_API static GameObject* CreateDebugSphere(Scene* s);
	CORE_API float GetGravityStrength() const;
	CORE_API GameMode* GetGameMode();
	CORE_API bool IsSceneDestorying()const;
	int FindAllOfName(std::string name, std::vector<GameObject*>& Objects);
	CORE_API GameObject * FindByName(std::string name);
	std::vector<GameObject*> ObjectsAddedLastFrame;
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

