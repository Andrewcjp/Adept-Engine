#pragma once
#if TDSIM_ENABLED
#include "Physics/Physics_fwd.h"
#include "Physics/PhysicsTypes.h"
#include "TDPhysicsAPI.h"
#include "Physics/GenericPhysicsEngine.h"
#include "Physics/GenericConstraint.h"

using namespace TD;
class GameObject;
struct ConstaintSetup;
class TDPhysicsEngine : public GenericPhysicsEngine
{
public:
	void initPhysics();
	CORE_API ConstraintInstance * CreateConstraint(RigidBody * A, RigidBody * B,const ConstaintSetup& Setup);
	static void DebugLineCallbackHandler(glm::vec3 start, glm::vec3 end, glm::vec3 Colour, float lifetime);
	static void TimerCallbackHandler(bool IsStart, TDPerfCounters::Type type);
	void stepPhysics(float Deltatime);
	void cleanupPhysics();
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * hit);
	CORE_API bool RayCastScene(glm::vec3 startpos, glm::vec3 direction, float distance, RayHit * outhit, std::vector<RigidBody*>& IgnoredActors);
	static TDScene* GetScene();
	static TDPhysicsEngine* Get();
	static EPhysicsDebugMode::Type GetCurrentMode();
	CORE_API ConstraintInstance * CreateConstraint(RigidBody * A, RigidBody * B, ConstaintSetup Setup);
private:
	TDScene* PlayScene = nullptr;
	TDScene* EditorScene = nullptr;
	static TDPhysicsEngine* Instance;
};
#endif
