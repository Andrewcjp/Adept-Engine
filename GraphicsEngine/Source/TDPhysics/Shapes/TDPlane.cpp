
#include "TDPlane.h"

namespace TD
{
	TDPlane::TDPlane(glm::vec3 normal) :TDPlane()
	{
		Normal = normal;
	}


	TDPlane::TDPlane() :TDShape()
	{
		ShapeType = TDShapeType::ePLANE;
	}

	TDPlane::~TDPlane()
	{}

	glm::vec3 TDPlane::GetNormal() const
	{
		return Normal;
	}

	glm::vec3 TDPlane::GetBoundBoxHExtents()
	{
		return glm::vec3(100000, 10, 1000000);
	}

	glm::vec3 TDPlane::ClosestPoint(const glm::vec3& point)
	{
		// This works assuming plane.Normal is normalized, which it should be
		float distance = glm::dot(Normal, point) - PlaneDistance;
		// If the plane normal wasn't normalized, we'd need this:
		// distance = distance / DOT(plane.Normal, plane.Normal);

		return point - Normal * distance;
	}

}
