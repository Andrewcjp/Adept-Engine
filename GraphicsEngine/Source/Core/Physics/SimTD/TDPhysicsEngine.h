#pragma once
#if TDSIM_ENABLED
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
#include "TDPhysicsAPI.h"
#include "Physics/GenericPhysicsEngine.h"

using namespace TD;
class GameObject;
class TDPhysicsEngine: public GenericPhysicsEngine
{
public:
	void initPhysics();
	static void TimerCallbackHandler(bool IsStart, TDPerfCounters::Type type);
	void stepPhysics(float Deltatime);
	void cleanupPhysics();

	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, std::vector<RigidBody*>& IgnoredActors);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, bool CastEdtiorScene, std::vector<RigidBody*>& IgnoredActors = std::vector<RigidBody*>());
	CORE_API RigidBody * CreatePrimitiveRigidBody(glm::vec3 position, glm::vec3 velocity, float scale);
	
	//old:
	void AddBoxCollisionToEditor(GameObject * obj);
	CORE_API std::vector<RigidBody*> createStack(const glm::vec3 & t, int size, float halfExtent);
	CORE_API RigidBody * FirePrimitiveAtScene(glm::vec3 position, glm::vec3 velocity, float scale/*, PxGeometryType::Enum type*/);
	static TDScene* GetScene();
	static TDPhysicsEngine* Get();
	static EPhysicsDebugMode::Type GetCurrentMode();
private:
	TDScene* PlayScene = nullptr;
	static TDPhysicsEngine* Instance;
};
#endif
