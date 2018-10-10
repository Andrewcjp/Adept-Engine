#pragma once
#if PHYSX_ENABLED
#include <vector>
#include "glm\glm.hpp"
#include "Physics/Physx/PhysxRigidbody.h"
#include "Physics/GenericPhysicsEngine.h"
#include <include/physx/PxPhysicsAPI.h>
#include "include/physx/foundation/PxSimpleTypes.h"
#include <iostream>

#include <vector>
#define PVD_HOST "127.0.0.1"
class PhysxCallBackHandler;
class PhysxEngine : public GenericPhysicsEngine
{
public:
	void initPhysics();
	void stepPhysics(float Deltatime);

	std::vector<physx::PxRigidActor*> GetActors();

	void cleanupPhysics();
	CORE_API std::vector<RigidBody*> createStack(const glm::vec3 & t, int size, float halfExtent);

	CORE_API RigidBody *FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale/*, PxGeometryType::Enum type*/);
	CORE_API RigidBody * CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, std::vector<RigidBody*>& IgnoredActors);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene, std::vector<RigidBody*>& IgnoredActors = std::vector<RigidBody*>());
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit);

	void AddBoxCollisionToEditor(GameObject * obj);

	physx::PxRigidDynamic * createDynamic(const physx::PxTransform & t, const physx::PxGeometry & geometry, const physx::PxVec3 & velocity);

	std::vector<physx::PxRigidDynamic*> createStack(const physx::PxTransform & t, physx::PxU32 size, physx::PxReal halfExtent);

	physx::PxRigidDynamic * CreateActor(physx::PxVec3 position, physx::PxReal halfextent, physx::PxGeometryType::Enum type = physx::PxGeometryType::eBOX);


	physx::PxRigidDynamic *FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale, physx::PxGeometryType::Enum type);



	static glm::vec3 PXvec3ToGLM(physx::PxVec3 val)
	{
		return glm::vec3(val.x, val.y, val.z);
	}
	//static glm::quat PXquatToGLM(physx::PxQuat val) {
	//	return glm::quat(val.x, val.y, val.z, val.w);
	//}
	static physx::PxVec3 GLMtoPXvec3(glm::vec3 val)
	{
		return physx::PxVec3(val.x, val.y, val.z);
	}

	static physx::PxPhysics* GetGPhysics();
	static physx::PxScene * GetPlayScene();
	static physx::PxMaterial* GetDefaultMaterial();
	physx::PxMaterial* CreatePhysxMat(PhysicalMaterial* mat);
private:
	class ColliderComponent*		DebugFloor = nullptr;
	PhysxCallBackHandler*			CallBackHandler = nullptr;
	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;

	physx::PxFoundation*			gFoundation = NULL;
	physx::PxPhysics*				gPhysics = NULL;

	physx::PxDefaultCpuDispatcher*	gDispatcher = NULL;
	physx::PxScene*				gScene = NULL;
	physx::PxScene*				gEdtiorScene = NULL;
	physx::PxMaterial*				gMaterial = NULL;


	physx::PxPvd*                  gPvd = NULL;

	physx::PxReal stackZ = 10.0f;

};
//typedef PhysxEngine PhysicsEngine;
#endif