#include "TDPCH.h"
#include "TDPlane.h"

namespace TD {
	TDPlane::TDPlane()
	{
		ShapeType = TDShapeType::ePLANE;
	}


	TDPlane::~TDPlane()
	{
	}

	glm::vec3 TDPlane::GetBoundBoxHExtents()
	{
		return glm::vec3(100000, 10, 1000000);
	}

}
