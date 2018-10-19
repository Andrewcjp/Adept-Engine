#include "TDCollisionHandlers.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDMesh.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDPlane.h"

bool TD::TDCollisionHandlers::CollideSphereSphere(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxMesh(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideConvexConvex(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideConvexMesh(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::InvalidCollisonPair(CollisionHandlerArgs)
{
	assert(false);
	return false;
}

bool TD::TDCollisionHandlers::CollideSpherePlane(CollisionHandlerArgs)
{
	TDSphere* Sphere = TDShape::CastShape<TDSphere>(A);

	TDTransform* ATransform = A->GetOwner()->GetTransfrom();
	TDTransform* BTransform = B->GetOwner()->GetTransfrom();
	glm::vec3 SpherePos = glm::inverse(BTransform->GetModel())*glm::vec4(ATransform->GetPos(), 0);
	//BTransform->TransfromToLocalSpace()
	const float Seperation = SpherePos.y - Sphere->Radius;
	if (Seperation <= 0.0f)
	{
		const glm::vec3 normal = BTransform->GetUp();
		const glm::vec3 point = ATransform->GetPos() - normal * Sphere->Radius;
		contactbuffer->depth = abs(Seperation);
		contactbuffer->ContactPoints[0] = point;
		contactbuffer->Blocking = true;
		contactbuffer->Direction = normal;
		return true;
	}
	return false;
}

bool TD::TDCollisionHandlers::CollideSphereCapsule(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideSphereBox(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideSphereConvex(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideSphereMesh(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollidePlaneCapsule(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollidePlaneBox(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollidePlaneConvex(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollidePlaneMesh(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideCapsuleCapsule(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideCapsuleBox(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideCapsuleConvex(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideCapsuleMesh(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxBox(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxConvex(CollisionHandlerArgs)
{
	return false;
}
