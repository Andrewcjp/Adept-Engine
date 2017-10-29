#pragma once
#include "Core/GameObject.h"
#include "SceneSerialiser.h"
#include "../Rendering/Core/Light.h"
#include <vector>
class Scene
{
public:
	Scene();
	~Scene();
	void UpdateScene(float deltatime);
	void FixedUpdateScene(float deltatime);
	void AddGameobjectToScene(GameObject* gameobject) {
		SceneObjects.push_back(gameobject);
	}
	std::vector<GameObject*>* GetObjects() { return &SceneObjects; };
	void LoadDefault();
	void StartScene();
	std::vector<Light*>* GetLights() { return &Lights; };
private:
	std::vector<GameObject*> SceneObjects;	
	std::vector<Light*> Lights;
	SceneSerialiser* serialiser;
};

