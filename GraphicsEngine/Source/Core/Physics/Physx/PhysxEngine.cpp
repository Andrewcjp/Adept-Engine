
#include "PhysxEngine.h"
#if PHYSX_ENABLED
#include <thread>
#include "Core/GameObject.h"
#include "Core/Engine.h"
#define ENABLEPVD !(NDEBUG)
#include <algorithm>
#include "PhysxSupport.h"
#include "Core/Components/ColliderComponent.h"
using namespace physx;
physx::PxFilterFlags CollisionFilterShader(
	physx::PxFilterObjectAttributes /*attributes0*/, physx::PxFilterData /*filterData0*/,
	physx::PxFilterObjectAttributes /*attributes1*/, physx::PxFilterData /*filterData1*/,
	physx::PxPairFlags& retPairFlags, const void* /*constantBlock*/, PxU32 /*constantBlockSize*/)
{
	retPairFlags = PxPairFlag::eCONTACT_DEFAULT;
	retPairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eNOTIFY_TOUCH_FOUND;
	return PxFilterFlag::eDEFAULT;
}
PhysxEngine * PhysxEngine::Get()
{
	return Engine::GetPhysEngineInstance();
}
void PhysxEngine::initPhysics()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
#if ENABLEPVD
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxCookingParams params(gPhysics->getTolerancesScale());
	params.meshWeldTolerance = 0.001f;
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.buildGPUData = true; //Enable GRB data being produced in cooking.
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, params);


	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -20.0f, 0.0f);
	CallBackHandler = new PhysxCallBackHandler();
	sceneDesc.simulationEventCallback = CallBackHandler;
	//todo!
	int cpucount = std::thread::hardware_concurrency();
	unsigned int threadsToCreate = std::max((int)1, cpucount - 2);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = CollisionFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	gEdtiorScene = gPhysics->createScene(sceneDesc);
#if ENABLEPVD
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
#endif
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.0f);
#if 1
	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	//gScene->addActor(*groundPlane);
#else
	GameObject* go = new GameObject();
	go->SetPosition(glm::vec3(0, 0, 0));
	go->SetRotation(glm::vec3(90, 0, 0));
	DebugFloor = new ColliderComponent();
	DebugFloor->SetCollisonShape(EShapeType::ePLANE);
	go->AttachComponent(DebugFloor);
	DebugFloor->SceneInitComponent();
	go->SetRotation(glm::vec3(90, 0, 0));
	//DebugFloor->
#endif
	Log::OutS << "Physx Initalised" << Log::OutS;
}

void PhysxEngine::stepPhysics(float Deltatime)
{
	
	//Deltatime
	gScene->simulate(Deltatime);
	gScene->fetchResults(true);
}

void PhysxEngine::cleanupPhysics()
{
	gScene->release();
	gEdtiorScene->release();
	gDispatcher->release();
	gPhysics->release();
#if ENABLEPVD
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
#endif
	gFoundation->release();
	SafeDelete(CallBackHandler);
}

physx::PxPhysics * PhysxEngine::GetGPhysics()
{
	return Engine::GetPhysEngineInstance()->gPhysics;
}
physx::PxScene * PhysxEngine::GetPlayScene()
{
	return Engine::GetPhysEngineInstance()->gScene;
}
physx::PxMaterial * PhysxEngine::GetDefaultMaterial()
{
	return Engine::GetPhysEngineInstance()->gMaterial;
}
physx::PxMaterial * PhysxEngine::CreatePhysxMat(PhysicalMaterial * mat)
{
	return gPhysics->createMaterial(mat->StaticFriction, mat->DynamicFirction, mat->Bouncyness);
}
physx::PxCooking * PhysxEngine::GetCooker()
{
	return Engine::GetPhysEngineInstance()->gCooking;
}
bool PhysxEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* outhit,std::vector<RigidBody*>& IgnoredActors)
{
	return RayCastScene(startpos, direction, distance, outhit, false, IgnoredActors);
}
bool PhysxEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, false);
}

ConstraintInstance * PhysxEngine::CreateConstraint(RigidBody * A, RigidBody * B, ConstaintSetup Setup)
{
	PxD6Joint* PD6Joint = PxD6JointCreate(PxGetPhysics(), A->GetActor(),PxTransform(GLMtoPXvec3(A->GetPosition()), GLMtoPXQuat(A->GetRotation())),
		B->GetActor(), PxTransform(GLMtoPXvec3(B->GetPosition()), GLMtoPXQuat(B->GetRotation())));

	if (PD6Joint == nullptr)
	{
		
		return false;
	}


	return nullptr;
}

void PhysxEngine::AddBoxCollisionToEditor(GameObject* obj)
{
	PxRigidStatic* st = gPhysics->createRigidStatic(PxTransform(GLMtoPXvec3(obj->GetTransform()->GetPos())));
	PxRigidActorExt::createExclusiveShape(*st, PxBoxGeometry(2, 2, 2), *gMaterial);

	gEdtiorScene->addActor(*st);
	//obj->SelectionShape = st;
}

bool PhysxEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* outhit, bool CastEdtiorScene, std::vector<RigidBody*>& IgnoredActors)
{
	PxRaycastBuffer hit;
	//const PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
	//PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
	//PxRaycastBuffer hit(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here
	bool cast;
	if (CastEdtiorScene)
	{
		cast = gEdtiorScene->raycast(GLMtoPXvec3(startpos), GLMtoPXvec3(direction), distance, hit, PxHitFlag::eDEFAULT);
	}
	else 
	{
		PxQueryFilterData fd;
		fd.flags |= PxQueryFlag::ePREFILTER; // note the OR with the default value
		FPxQueryFilterCallback* filter = new FPxQueryFilterCallback();
		filter->IgnoredBodies = IgnoredActors;
		cast = gScene->raycast(GLMtoPXvec3(startpos), GLMtoPXvec3(direction), distance, hit, PxHitFlags(PxHitFlag::eDEFAULT), fd, filter);
		delete filter;
	}
	if (hit.hasBlock)
	{
		outhit->position = PXvec3ToGLM(hit.getAnyHit(0).position);
		outhit->HitBody = (RigidBody*)hit.block.actor->userData;
		outhit->Distance = hit.getAnyHit(0).distance;
		return true;
	}
	return false;
}

#endif