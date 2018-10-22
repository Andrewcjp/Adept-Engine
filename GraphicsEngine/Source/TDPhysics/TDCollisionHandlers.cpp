#include "TDCollisionHandlers.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDMesh.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDPlane.h"
bool TD::TDCollisionHandlers::InvalidCollisonPair(CollisionHandlerArgs)
{
	assert(false);
	return false;
}

//convex
bool TD::TDCollisionHandlers::CollideConvexConvex(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideConvexMesh(CollisionHandlerArgs)
{
	return false;
}

//Spheres 
bool TD::TDCollisionHandlers::CollideSphereSphere(CollisionHandlerArgs)
{
	TDSphere* SphereA = TDShape::CastShape<TDSphere>(A);
	TDSphere* SphereB = TDShape::CastShape<TDSphere>(B);

	glm::vec3 CollisonNormal = SphereA->GetOwner()->GetTransfrom()->GetPos() - SphereB->GetOwner()->GetTransfrom()->GetPos();
	const float Distancesq = glm::length2(CollisonNormal);
	const float RaduisSum = SphereA->Radius + SphereB->Radius;
	const float inflatedSum = RaduisSum + 0.0f;//mContactDistance
	if (Distancesq >= inflatedSum * inflatedSum)
	{
		return false;
	}
	float magn = glm::sqrt(Distancesq);
	//this would normally be a simple normalization of the CollisonNormal but the small number case has to handled here.
	if (magn <= 0.00001f)//the sphere are almost completely overlapped 
	{
		CollisonNormal = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else
	{
		CollisonNormal *= 1.0 / magn;
	}
	const glm::vec3 contactpoint = CollisonNormal * ((SphereA->Radius + magn - SphereB->Radius)*-0.5f) + SphereB->GetOwner()->GetTransfrom()->GetPos();
	contactbuffer->Contact(contactpoint, CollisonNormal, abs(magn - RaduisSum));
	return true;
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
		contactbuffer->Contact(point, normal, abs(Seperation));
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

//Plane
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

//Capsule
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

//Box
bool TD::TDCollisionHandlers::CollideBoxBox(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxConvex(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxMesh(CollisionHandlerArgs)
{
	return false;
}