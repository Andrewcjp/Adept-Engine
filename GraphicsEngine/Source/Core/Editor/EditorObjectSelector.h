#pragma once
#include <vector>
class GameObject;
class PhysicsEngine;
class Camera;
class EditorObjectSelector
{
public:
	EditorObjectSelector();
	~EditorObjectSelector();
	void init();
	void LinkPhysxBodysToGameObjects(const std::vector<GameObject*>& objects);
	GameObject * RayCastScene(int x, int y, Camera * cam, const std::vector<GameObject*>& objects);
private:
	PhysicsEngine* pengine;
};

