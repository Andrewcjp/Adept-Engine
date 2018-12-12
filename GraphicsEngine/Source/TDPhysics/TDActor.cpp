#include "TDActor.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDAABB.h"
#include "Utils/MemoryUtils.h"
#include "Shapes/TDMeshShape.h"
namespace TD
{

	TDActor::TDActor()
	{
		AABB = new TDAABB();
		AABB->Owner = this;
	}

	TDActor::~TDActor()
	{
		MemoryUtils::DeleteVector(AttachedShapes);
	}

	void TDActor::Init()
	{
		UpdateAABBPos(Transform.GetPos());
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
		SafeDelete(AABB);
		MemoryUtils::DeleteVector(AttachedShapes);
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
		if (newShape->GetShapeType() == TDShapeType::eTRIANGLEMESH)
		{
			AABB = ((TDMeshShape*)newShape)->GetAABB();
			AABB->Owner = this;
			LocalAABBPos = AABB->Position;
		}
	}

	std::vector<TDShape*>& TDActor::GetAttachedShapes()
	{
		return AttachedShapes;
	}

#if VALIDATE_KE
	void TDActor::ComputeKE()
	{}

	void TDActor::ValidateKE()
	{}
#endif

	void TDActor::UpdateAABBPos(glm::vec3 pos)
	{
		if (AABB != nullptr) 
		{
			AABB->Position = pos + LocalAABBPos;
		}
	}
}