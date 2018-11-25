#pragma once
#include "Physics/Physics_fwd.h"

#include "Physics/GenericRigidBody.h"
#include "Physics/GenericPhysicsEngine.h"

#if PHYSX_ENABLED
#include "Physics/Physx/PhysxRigidbody.h"
#include "Physics/Physx/PhysxEngine.h"
#endif
#if TDSIM_ENABLED
#include "Physics/SimTD/TDRigidBody.h"
#include "Physics/SimTD/TDPhysicsEngine.h"
#endif