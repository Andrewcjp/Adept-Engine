#pragma once
#if PHYSX_ENABLED
class PhysxRigidbody;
class PhysxEngine;
class PhysxCollider;
class PhysxConstraint;
typedef PhysxRigidbody RigidBody;
typedef PhysxEngine PhysicsEngine;
typedef PhysxCollider Collider;
typedef PhysxConstraint ConstraintInstance;
#elif TDSIM_ENABLED
class TDRigidBody;
class TDPhysicsEngine;
typedef TDRigidBody RigidBody;
typedef TDPhysicsEngine PhysicsEngine;
class GenericCollider;
typedef GenericCollider Collider;
class GenericConstraint;
typedef PhysxConstraint ConstraintInstance;
#endif
