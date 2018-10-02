#pragma once
#if TDSIM_ENABLED
#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include "Physics/Physics_fwd.h"
#include "Physics/GenericRigidBody.h"
#include "core/Transform.h"
#include "TDPhysicsAPI.h"
using namespace TD;
class TDRigidBody :public GenericRigidBody
{
public:
	TDRigidBody(EBodyType::Type type,Transform T);
	~TDRigidBody();
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3 force, EForceMode::Type Mode = EForceMode::AsForce);
	CORE_API glm::vec3 GetLinearVelocity();
	void AttachCollider(Collider* col);
	CORE_API void SetLockFlags(BodyInstanceData data);
	CORE_API BodyInstanceData GetLockFlags();
	CORE_API void SetLinearVelocity(glm::vec3 velocity);
	CORE_API void InitBody();
private:
	Transform m_transform;
	TDRigidDynamic* Actor = nullptr;
};

#endif