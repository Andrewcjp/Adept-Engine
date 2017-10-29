#pragma once
#include "EngineGlobals.h"
#if PHYSX_ENABLED
#include "PhysicsEngine.h"
//#include "include\physx\PxPhysicsAPI.h"
#include <include/physx/PxPhysicsAPI.h>

#include "include/physx/foundation/PxSimpleTypes.h"
#include "include\glm\glm.hpp"
#include <iostream>
using namespace physx;
#include <vector>
#define PVD_HOST "127.0.0.1"
class GameObject;
class PhysxEngine : public PhysicsEngine
{
public:

	~PhysxEngine();
	/*static void Init() {
		if (instance == nullptr) {
			instance = new PhysxEngine();
			instance->initPhysics(false);
		}
	}
	static PhysxEngine* instance;*/
	void initPhysics(bool interactive) override;
	void stepPhysics(bool interactive, float Deltatime)override;

	std::vector<PxRigidActor*> GetActors();

	void cleanupPhysics(bool interactive) override;
	std::vector<RigidBody*> createStack(const glm::vec3 & t, int size, float halfExtent)override ;

	RigidBody *FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale/*, PxGeometryType::Enum type*/)override;
	bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene);
	bool RayCastEditorScene(glm::vec3 startpos, glm::vec3 direction, float distance, PxRaycastBuffer * outhit);
	bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)override;

	void AddBoxCollisionToEditor(GameObject * obj);

	PxRigidDynamic * createDynamic(const PxTransform & t, const PxGeometry & geometry, const PxVec3 & velocity);

	std::vector<PxRigidDynamic*> createStack(const PxTransform & t, PxU32 size, PxReal halfExtent);

	PxRigidDynamic * CreateActor(PxVec3 position, PxReal halfextent, PxGeometryType::Enum type = PxGeometryType::eBOX);


	PxRigidDynamic *FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale, PxGeometryType::Enum type);



	static glm::vec3 PXvec3ToGLM(physx::PxVec3 val) {
		return glm::vec3(val.x, val.y, val.z);
	}
	//static glm::quat PXquatToGLM(physx::PxQuat val) {
	//	return glm::quat(val.x, val.y, val.z, val.w);
	//}
	static physx::PxVec3 GLMtoPXvec3(glm::vec3 val) {
		return physx::PxVec3(val.x, val.y, val.z);
	}


private:

	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;

	PxFoundation*			gFoundation = NULL;
	PxPhysics*				gPhysics = NULL;

	PxDefaultCpuDispatcher*	gDispatcher = NULL;
	PxScene*				gScene = NULL;
	PxScene*				gEdtiorScene = NULL;
	PxMaterial*				gMaterial = NULL;


	PxPvd*                  gPvd = NULL;

	PxReal stackZ = 10.0f;
};

#endif