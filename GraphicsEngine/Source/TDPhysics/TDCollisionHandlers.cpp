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

bool TD::TDCollisionHandlers::CollideBoxSphere(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideMeshSphere(CollisionHandlerArgs)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxMesh(CollisionHandlerArgs)
{
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
		contactbuffer->ContactPoints[0] = point;
		return true;
	}
	return false;
}
