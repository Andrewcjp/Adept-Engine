#include "TDCollisionHandlers.h"


bool TD::TDCollisionHandlers::CollideSphereSphere(TDSphere * A, TDSphere * B, ContactData * hitbuffer)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxSphere(TDSphere * A, TDBox * B, ContactData * htibuffer)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideMeshSphere(TDSphere * A, TDMesh * B, ContactData * hitbuffer)
{
	return false;
}

bool TD::TDCollisionHandlers::CollideBoxMesh(TDBox * A, TDMesh * B, ContactData * hitbuffer)
{
	return false;
}

bool TD::TDCollisionHandlers::CollidePlaneSphere(TDPlane * A, TDSphere * B, ContactData * hitbuffer)
{
	TDTransform* ATransform = A->GetOwner()->GetTransfrom();
	TDTransform* BTransform = B->GetOwner()->GetTransfrom();
	glm::vec3 SpherePos = glm::inverse(ATransform->GetModel())*glm::vec4(BTransform->GetPos(), 0);
	//BTransform->TransfromToLocalSpace()
	const float Seperation = SpherePos.x - B->Radius;
	if (Seperation <= 0.0f) 
	{
		const glm::vec3 normal = ATransform->GetUp();
		const glm::vec3 point = BTransform->GetPos() - normal * B->Radius;
		hitbuffer->ContractPoints[0] = point;
		return true;
	}
	return false;
}
