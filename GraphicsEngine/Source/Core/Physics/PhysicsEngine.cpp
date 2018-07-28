#include "PhysicsEngine.h"
#include "RigidBody.h"
#include <thread>
#include "Core/GameObject.h"
#define ENABLEPVD !(NDEBUG)
#include <algorithm>
using namespace physx;
void PhysicsEngine::initPhysics()
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
	//todo!
	int cpucount = std::thread::hardware_concurrency();
	unsigned int threadsToCreate = std::max(( int)1, cpucount - 2);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
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
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);
	Log::OutS  << "Physx Initalised" << Log::OutS;
}

void PhysicsEngine::stepPhysics(float Deltatime)
{
	//Deltatime
	gScene->simulate(Deltatime);
	gScene->fetchResults(true);
}
std::vector<PxRigidActor*> PhysicsEngine::GetActors()
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
void PhysicsEngine::cleanupPhysics()
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
}
std::vector<RigidBody*> PhysicsEngine::createStack(const glm::vec3 & t, int size, float halfExtent)
{
	std::vector<PxRigidDynamic*> objects = createStack(PxTransform(GLMtoPXvec3(t)), size, halfExtent);
	std::vector<RigidBody*> Bodies;
	for (int i = 0; i < objects.size(); i++)
	{
		RigidBody* body = new RigidBody(objects[i]);
		Bodies.push_back(body);
	}
	return Bodies;
}
PxRigidDynamic* PhysicsEngine::createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
	return dynamic;


}

std::vector<PxRigidDynamic*> PhysicsEngine::createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
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

PxRigidDynamic* PhysicsEngine::CreateActor(PxVec3 position, PxReal halfextent, PxGeometryType::Enum type)
{

	PxShape* shape;
	switch (type)
	{
	case physx::PxGeometryType::eSPHERE:
		shape = gPhysics->createShape(PxSphereGeometry(halfextent), *gPhysics->createMaterial(0.1f, 0.05f, 0.6f));
		break;
	case physx::PxGeometryType::eBOX:
		shape = gPhysics->createShape(PxBoxGeometry(halfextent, halfextent, halfextent), *gPhysics->createMaterial(0.1f, 0.05f, 0.6f));
		break;
	case physx::PxGeometryType::ePLANE:
	case physx::PxGeometryType::eCAPSULE:
	case physx::PxGeometryType::eCONVEXMESH:
	case physx::PxGeometryType::eTRIANGLEMESH:
	case physx::PxGeometryType::eHEIGHTFIELD:
	case physx::PxGeometryType::eGEOMETRY_COUNT:
	case physx::PxGeometryType::eINVALID:
	default:
		shape = gPhysics->createShape(PxSphereGeometry(halfextent), *gPhysics->createMaterial(0.1f, 0.05f, 0.6f));
		break;
	}

	PxRigidDynamic* body = gPhysics->createRigidDynamic(PxTransform(position));
	body->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*body, 5.0f);
	gScene->addActor(*body);

	body->setAngularDamping(1);
	//body->set
	return body;
}
PxRigidDynamic* PhysicsEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale, PxGeometryType::Enum type)
{
	PxRigidDynamic* newobject = CreateActor(GLMtoPXvec3(position), scale, type);
	newobject->addForce(GLMtoPXvec3(velocity), PxForceMode::eFORCE, true);
	return newobject;
}
RigidBody * PhysicsEngine::FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale)
{
	return new RigidBody(FirePrimitiveAtScene(position, velocity, scale, PxGeometryType::eSPHERE));
}
RigidBody* PhysicsEngine::CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale)
{
	PX_UNUSED(velocity);
	return new RigidBody(CreateActor(GLMtoPXvec3(position), scale, PxGeometryType::eSPHERE));
}
bool PhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* hit)
{
	return RayCastScene(startpos, direction, distance, hit, false);
}
void PhysicsEngine::AddBoxCollisionToEditor(GameObject* obj)
{
	PxRigidStatic* st = gPhysics->createRigidStatic(PxTransform(GLMtoPXvec3(obj->GetTransform()->GetPos())));
	PxRigidActorExt::createExclusiveShape(*st, PxBoxGeometry(2, 2, 2), *gMaterial);

	gEdtiorScene->addActor(*st);
	obj->SelectionShape = st;

}
bool PhysicsEngine::RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit* outhit, bool CastEdtiorScene)
{
	PxRaycastBuffer hit;

	bool cast;
	if (CastEdtiorScene)
	{
		cast = gEdtiorScene->raycast(GLMtoPXvec3(startpos), GLMtoPXvec3(direction), distance, hit);
	}
	else
	{
		cast = gScene->raycast(GLMtoPXvec3(startpos), GLMtoPXvec3(direction), distance, hit);
	}
	if (cast)
	{
		outhit->position = PXvec3ToGLM(hit.getAnyHit(0).position);
		return true;
	}
	return false;
}
bool PhysicsEngine::RayCastEditorScene(glm::vec3 startpos, glm::vec3 direction, float distance, PxRaycastBuffer* outhit)
{
	PxRaycastBuffer hit;

	bool cast;

	cast = gEdtiorScene->raycast(GLMtoPXvec3(startpos), GLMtoPXvec3(direction), distance, *outhit);

	if (cast)
	{
		//outhit->position = PXvec3ToGLM(hit.getAnyHit(0).position);
		return true;
	}
	return false;
}
