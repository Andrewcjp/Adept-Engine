#include "Stdafx.h"
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
void PhysxEngine::initPhysics()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
#if ENABLEPVD
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
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
	gScene->addActor(*groundPlane);
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
std::vector<PxRigidActor*> PhysxEngine::GetActors()
{
	PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if (nbActors > 0)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, (PxActor**)&actors[0], nbActors);
		return actors;
		//Snippets::renderActors(&actors[0], (PxU32)actors.size(), true);
	}
	std::vector<PxRigidActor*> actors;
	return actors;
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
std::vector<RigidBody*> PhysxEngine::createStack(const glm::vec3 & t, int size, float halfExtent)
{
	std::vector<PxRigidDynamic*> objects = createStack(PxTransform(GLMtoPXvec3(t)), size, halfExtent);
	std::vector<RigidBody*> Bodies;
	for (int i = 0; i < objects.size(); i++)
	{
		//		RigidBody* body = new PhysxRigidbody(objects[i]);
				//Bodies.push_back(body);
	}
	return Bodies;
}
PxRigidDynamic* PhysxEngine::createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
	return dynamic;


}

std::vector<PxRigidDynamic*> PhysxEngine::createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	std::vector<PxRigidDynamic*> objects;
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
			objects.push_back(body);
		}
	}
	shape->release();
	return objects;
}

PxRigidDynamic* PhysxEngine::CreateActor(PxVec3 position, PxReal halfextent, PxGeometryType::Enum type)
{

	//PxShape* shape;
	//switch (type)
	//{
	//case physx::PxGeometryType::eSPHERE:
	//	shape = gPhysics->createShape(PxSphereGeometry(halfextent), *gPhysics->createMaterial(0.1f, 0.05f, 0.6f));
	//	break;
	//case physx::PxGeometryType::eBOX:
	//	shape = gPhysics->createShape(PxBoxGeometry(halfextent, halfextent, halfextent), *gPhysics->createMaterial(0.1f, 0.05f, 0.6f));
	//	break;
	//case physx::PxGeometryType::ePLANE:
	//case physx::PxGeometryType::eCAPSULE:
	//case physx::PxGeometryType::eCONVEXMESH:
	//case physx::PxGeometryType::eTRIANGLEMESH:
	//case physx::PxGeometryType::eHEIGHTFIELD:
	//case physx::PxGeometryType::eGEOMETRY_COUNT:
	//case physx::PxGeometryType::eINVALID:
	//default:
	//	shape = gPhysics->createShape(PxSphereGeometry(halfextent), *gPhysics->createMaterial(0.1f, 0.05f, 0.6f));
	//	break;
	//}




	////body->set
	//return body;
	return nullptr;
}
PxRigidDynamic* PhysxEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale, PxGeometryType::Enum type)
{
	PxRigidDynamic* newobject = CreateActor(GLMtoPXvec3(position), scale, type);
	newobject->addForce(GLMtoPXvec3(velocity), PxForceMode::eFORCE, true);
	return newobject;
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
RigidBody * PhysxEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return nullptr;/// new PhysxRigidbody(FirePrimitiveAtScene(position, velocity, scale, PxGeometryType::eSPHERE));
}
RigidBody* PhysxEngine::CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale)
{
	PX_UNUSED(velocity);
	return nullptr;// new PhysxRigidbody(CreateActor(GLMtoPXvec3(position), scale, PxGeometryType::eSPHERE));
}
bool PhysxEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, false);
}
void PhysxEngine::AddBoxCollisionToEditor(GameObject* obj)
{
	PxRigidStatic* st = gPhysics->createRigidStatic(PxTransform(GLMtoPXvec3(obj->GetTransform()->GetPos())));
	PxRigidActorExt::createExclusiveShape(*st, PxBoxGeometry(2, 2, 2), *gMaterial);

	gEdtiorScene->addActor(*st);
	//obj->SelectionShape = st;
}

bool PhysxEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* outhit, bool CastEdtiorScene)
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
		//fd.flags |= PxQueryFlag::; // note the OR with the default value
		cast = gScene->raycast(GLMtoPXvec3(startpos), GLMtoPXvec3(direction), distance, hit, PxHitFlags(PxHitFlag::eDEFAULT),fd);
	}
	if (hit.hasBlock)
	{
		outhit->position = PXvec3ToGLM(hit.getAnyHit(0).position);
		outhit->HitBody = (RigidBody*)hit.block.actor->userData;
		return true;
	}
	return false;
}

#endif