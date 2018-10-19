#include "TDActor.h"
#include "TDShape.h"
namespace TD
{

	TDActor::TDActor()
	{}

	TDActor::~TDActor()
	{}

	void TDActor::Init()
	{

	}

	TDActorType::Type TDActor::GetActorType() const
	{
		return ActorType;
	}

	TDScene * TDActor::GetScene() const
	{
		return OwningScene;
	}

	void TDActor::Release()
	{

	}
	TDTransform * TDActor::GetTransfrom()
	{
		return &Transform;
	}
	float TDActor::GetBodyMass()
	{
		return BodyMass;
	}
	float TDActor::GetInvBodyMass()
	{
		if (BodyMass <= FLT_EPSILON)
		{
			return 0.0f;
		}
		return 1.0f / BodyMass;
	}
	glm::vec3 TDActor::GetVelocityDelta()
	{
		return glm::vec3();
	}
	glm::vec3 TDActor::GetLinearVelocity()
	{
		return LinearVelocity;
	}
	void TDActor::SetLinearVelocity(glm::vec3 newvel)
	{
		if (GetActorType() != TDActorType::RigidStatic)
		{
			LinearVelocity = newvel;
		}
	}
	void TDActor::AttachShape(TDShape * newShape)
	{
		AttachedShapes.push_back(newShape);
		newShape->SetOwner(this);
	}
	std::vector<TDShape*>& TDActor::GetAttachedShapes()
	{
		return AttachedShapes;
	}
}