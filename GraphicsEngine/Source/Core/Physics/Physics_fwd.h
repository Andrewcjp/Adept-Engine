#pragma once
class GenericCollider;
#if PHYSX_ENABLED
class PhysxRigidbody;
class PhysxEngine;
class PhysxCollider;
class PhysxConstraint;
typedef PhysxRigidbody RigidBody;
typedef PhysxEngine PhysicsEngine;
typedef GenericCollider Collider;
typedef PhysxConstraint ConstraintInstance;
#elif TDSIM_ENABLED
class TDRigidBody;
class TDPhysicsEngine;
typedef TDRigidBody RigidBody;
typedef TDPhysicsEngine PhysicsEngine;
typedef GenericCollider Collider;
class TD_ConstraintInstance;
typedef TD_ConstraintInstance ConstraintInstance;
#else
class GenericRigidBody;
class GenericPhysicsEngine;
class GenericConstraint;
typedef GenericRigidBody RigidBody;
typedef GenericPhysicsEngine PhysicsEngine;
typedef GenericCollider Collider;
typedef GenericConstraint ConstraintInstance;
#endif
