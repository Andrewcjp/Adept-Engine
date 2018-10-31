#pragma once
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
class GameObject;
/*! This class is used as the compile time base class for the rigid body class as only one physics engine can be used at once*/
class GenericPhysicsEngine
{
public:
	void initPhysics();
	void stepPhysics(float Deltatime);
	void cleanupPhysics();

	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit);
	bool GetShowPhysicsShape();

	//old:
	void AddBoxCollisionToEditor(GameObject * obj);
	
private:
	bool ShowPhysicsDebugShapes = false;
	 
};

