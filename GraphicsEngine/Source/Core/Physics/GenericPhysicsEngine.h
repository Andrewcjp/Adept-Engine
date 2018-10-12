#pragma once
#include <vector>
#include "glm\glm.hpp"
#include <iostream>
#include <vector>
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
class GameObject;
class GenericPhysicsEngine
{
public:
	void initPhysics();
	void stepPhysics(float Deltatime);
	void cleanupPhysics();

	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit);

	//old:
	void AddBoxCollisionToEditor(GameObject * obj);
private:


};

