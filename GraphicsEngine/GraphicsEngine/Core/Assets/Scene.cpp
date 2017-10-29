#include "Scene.h"



Scene::Scene()
{
	serialiser = new SceneSerialiser(this);
	//serialiser->load("../asset/scene/Data.txt");
}


Scene::~Scene()
{
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

void Scene::FixedUpdateScene(float deltatime)
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->FixedUpdate(deltatime);
	}
}

void Scene::LoadDefault()
{
}

void Scene::StartScene()
{
	for (int i = 0; i < SceneObjects.size(); i++)
	{
		SceneObjects[i]->BeginPlay();
	}
}
