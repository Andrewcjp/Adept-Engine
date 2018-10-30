#include "TDActor.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDAABB.h"
namespace TD
{

	TDActor::TDActor()
	{
		BroadPhaseShape = new TDSphere();
		BroadPhaseShape->Radius = 1.0f;//todo:
		BroadPhaseShape->SetOwner(this);
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
		AttachedShapes.push_back(newShape);
		newShape->SetOwner(this);
		TDSphere* shape = TDShape::CastShape<TDSphere>(newShape);
		if (shape != nullptr)
		{
			BroadPhaseShape->Radius += shape->Radius;
			AABB->HalfExtends = glm::vec3(shape->Radius, shape->Radius, shape->Radius) * 2;
		}
		if (newShape->GetShapeType() == TDShapeType::ePLANE)
		{
			AABB->HalfExtends = glm::vec3(100000, 10, 1000000);
		}
	}
	std::vector<TDShape*>& TDActor::GetAttachedShapes()
	{
		return AttachedShapes;
	}


}