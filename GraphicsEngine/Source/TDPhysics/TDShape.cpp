
#include "TDShape.h"

namespace TD
{
	TDShape::TDShape()
	{
		Flags.SetFlagValue(TDShapeFlags::ESimulation, true);
	}


	TDShape::~TDShape()
	{}

	void TDShape::SetPhysicalMaterial(TDPhysicalMaterial mat)
	{
		ShapeMaterial = mat;
	}

	const TDPhysicalMaterial * TDShape::GetPhysicalMaterial() const
	{
		return &ShapeMaterial;
	}

	TDShapeType::Type TDShape::GetShapeType() const
	{ 
		DebugEnsure((ShapeType != TDShapeType::eLimit));
		return ShapeType;
	}

	void TDShape::SetOwner(TDActor * newowner)
	{
		Owner = newowner;
		Transfrom.SetParent(Owner->GetTransfrom());
	}

	TDActor * TDShape::GetOwner()
	{
		return Owner;
	}

	glm::vec3 TDShape::GetBoundBoxHExtents()
	{
		return glm::vec3(1, 1, 1);
	}

	glm::vec3 TDShape::GetPos()
	{
#if 0
		if (GetOwner() != nullptr)
		{
			return GetOwner()->GetTransfrom()->GetPos();
		}
		return glm::vec3();
#else
		return Transfrom.GetPos();
#endif
	}

	TD::TDTransform* TDShape::GetTransfrom()
	{
		return &Transfrom;
	}

}