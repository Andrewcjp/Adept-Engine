#pragma once
#if PHYSX_ENABLED

#include "Physics/Physx/PhysxRigidbody.h"
#include "Physics/GenericPhysicsEngine.h"
#include "Physics/GenericConstraint.h"
#include <include/physx/PxPhysicsAPI.h>
#include "include/physx/foundation/PxSimpleTypes.h"


#define PVD_HOST "127.0.0.1"
class PhysxCallBackHandler;
class PhysxEngine : public GenericPhysicsEngine
{
public:
	void initPhysics();
	void stepPhysics(float Deltatime);

	void cleanupPhysics();

	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, std::vector<RigidBody*>& IgnoredActors);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene, std::vector<RigidBody*>& IgnoredActors = std::vector<RigidBody*>());
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit);
	CORE_API ConstraintInstance* CreateConstraint(RigidBody* A, RigidBody* B, ConstaintSetup Setup);
	void AddBoxCollisionToEditor(GameObject * obj);

	static glm::vec3 PXvec3ToGLM(physx::PxVec3 val)
	{
		return glm::vec3(val.x, val.y, val.z);
	}
	static glm::quat PXquatToGLM(physx::PxQuat val)
	{
		return glm::quat(val.x, val.y, val.z, val.w);
	}
	static physx::PxVec3 GLMtoPXvec3(glm::vec3 val)
	{
		return physx::PxVec3(val.x, val.y, val.z);
	}
	static physx::PxQuat GLMtoPXQuat(glm::quat val)
	{
		return physx::PxQuat(val.x, val.y, val.z, val.w);
	}
	static physx::PxPhysics* GetGPhysics();
	static physx::PxScene * GetPlayScene();
	static physx::PxMaterial* GetDefaultMaterial();
	physx::PxMaterial* CreatePhysxMat(PhysicalMaterial* mat);
	static 	physx::PxCooking* GetCooker();
private:
	class ColliderComponent*		DebugFloor = nullptr;
	PhysxCallBackHandler*			CallBackHandler = nullptr;
	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;

	physx::PxFoundation*			gFoundation = NULL;
	physx::PxPhysics*				gPhysics = NULL;
	physx::PxCooking*				gCooking = NULL;
	physx::PxDefaultCpuDispatcher*	gDispatcher = NULL;
	physx::PxScene*				gScene = NULL;
	physx::PxScene*				gEdtiorScene = NULL;
	physx::PxMaterial*				gMaterial = NULL;


	physx::PxPvd*                  gPvd = NULL;

	physx::PxReal stackZ = 10.0f;

};
#endif