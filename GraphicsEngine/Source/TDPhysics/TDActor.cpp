#include "TDActor.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDAABB.h"
namespace TD
{

	TDActor::TDActor()
	{
		AABB = new TDAABB();
		AABB->Owner = this;
	}

	TDActor::~TDActor()
	{}

	void TDActor::Init()
	{
		AABB->Position = Transform.GetPos();
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

	void TDActor::AttachShape(TDShape * newShape)
	{
		if (newShape == nullptr)
		{
			return;
		}
		AttachedShapes.push_back(newShape);
		newShape->SetOwner(this);
		AABB->HalfExtends = newShape->GetBoundBoxHExtents();
	}

	std::vector<TDShape*>& TDActor::GetAttachedShapes()
	{
		return AttachedShapes;
	}
}