#pragma once
#if PHYSX_ENABLED
class PhysxRigidbody;
class PhysxEngine;
class PhysxCollider;
typedef PhysxRigidbody RigidBody;
typedef PhysxEngine PhysicsEngine;
typedef PhysxCollider Collider;
#elif TDSIM_ENABLED
class TDRigidBody;
class TDPhysicsEngine;
typedef TDRigidBody RigidBody;
typedef TDPhysicsEngine PhysicsEngine;
class GenericCollider;
typedef GenericCollider Collider;
#endif
