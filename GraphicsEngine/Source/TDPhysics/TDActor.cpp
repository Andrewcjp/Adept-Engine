#include "TDActor.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
namespace TD
{

	TDActor::TDActor()
	{
		BroadPhaseShape = new TDSphere();
		BroadPhaseShape->Radius = 1.0f;//todo:
		BroadPhaseShape->SetOwner(this);
	}

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

	void TDActor::AttachShape(TDShape * newShape)
	{
		AttachedShapes.push_back(newShape);
		newShape->SetOwner(this);
		TDSphere* shape = TDShape::CastShape<TDSphere>(newShape);
		if (shape != nullptr)
		{
			BroadPhaseShape->Radius += shape->Radius;
		}
	}
	std::vector<TDShape*>& TDActor::GetAttachedShapes()
	{
		return AttachedShapes;
	}


}