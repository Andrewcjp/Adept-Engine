#include "TDPCH.h"
#include "TDSphere.h"
namespace TD
{

	TDSphere::TDSphere()
	{
		ShapeType = TDShapeType::eSPHERE;
	}

	TDSphere::~TDSphere()
	{}

	glm::vec3 TDSphere::GetBoundBoxHExtents()
	{
		return glm::vec3(Radius, Radius, Radius);
	}
}