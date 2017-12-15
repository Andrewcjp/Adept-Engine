#pragma once
#include <vector>
class GameObject;
class PhysxEngine;
class Camera;
class EditorObjectSelector
{
public:
	EditorObjectSelector();
	~EditorObjectSelector();
	void init();
	void LinkPhysxBodysToGameObjects(const std::vector<GameObject*>& objects);
	GameObject * RayCastScene(int x, int y, Camera * cam, const std::vector<GameObject*>& objects);
	//GameObject * RayCastScene(int x, int y, glm::vec3 campos, glm::vec3 camforward, std::vector<GameObject*>& objects);
//	GameObject * RayCastScene(int x, int y, glm::vec3 campos, glm::vec3 camforward);
private:
	PhysxEngine* pengine;
};

