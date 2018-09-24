#pragma once
#if PHYSX_ENABLED
class PhysxRigidbody;
class PhysxEngine;
typedef PhysxRigidbody RigidBody;
typedef PhysxEngine PhysicsEngine;
#elif TDSIM_ENABLED
class TDRigidBody;
class TDPhysicsEngine;
typedef TDRigidBody RigidBody;
typedef TDPhysicsEngine PhysicsEngine;
#endif
