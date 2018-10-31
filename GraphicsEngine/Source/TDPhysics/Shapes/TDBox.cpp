#include "TDPCH.h"
#include "TDBox.h"
namespace TD
{

	TDBox::TDBox()
	{
		ShapeType = TDShapeType::eBOX;
	}

	TDBox::~TDBox()
	{}

	glm::vec3 TDBox::GetBoundBoxHExtents()
	{
		return HalfExtends;
	}
}