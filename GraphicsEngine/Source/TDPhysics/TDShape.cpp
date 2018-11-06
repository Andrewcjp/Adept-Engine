
#include "TDShape.h"

namespace TD
{
	TDShape::TDShape()
	{}


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
		return ShapeType;
	}

	void TDShape::SetOwner(TDActor * newowner)
	{
		Owner = newowner;
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
		if (GetOwner() != nullptr)
		{
			return GetOwner()->GetTransfrom()->GetPos();
		}
		return glm::vec3();
	}
}