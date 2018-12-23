#include "Source/TDPhysics/TDPCH.h"
#include "TDSAT.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDAABB.h"
namespace TD
{

	Interval TDSAT::GetInterval(TDBox* obb, const glm::vec3& axis)
	{
		glm::vec3 vertex[8];

		glm::vec3 C = obb->GetPos();	// OBB Center
		glm::vec3 E = obb->HalfExtends;		// OBB Extents
		//const float* o = obb->GetTransfrom;
		//glm::vec3 A[] = {			// OBB Axis
		//	glm::vec3(o[0], o[1], o[2]),
		//	glm::vec3(o[3], o[4], o[5]),
		//	glm::vec3(o[6], o[7], o[8]),
		//};
		glm::mat3x3 A = glm::mat3(obb->GetTransfrom()->GetQuatRot());
		glm::vec3 u0 = glm::vec3(A[0][0], A[1][0], A[2][0]);
		glm::vec3 u1 = glm::vec3(A[0][1], A[1][1], A[2][1]);//todo: wrong way round?
		glm::vec3 u2 = glm::vec3(A[0][2], A[1][2], A[2][2]);

		vertex[0] = C + A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
		vertex[1] = C - A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
		vertex[2] = C + A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
		vertex[3] = C + A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
		vertex[4] = C - A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
		vertex[5] = C + A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
		vertex[6] = C - A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
		vertex[7] = C - A[0] * E[0] - A[1] * E[1] + A[2] * E[2];

		Interval result;
		result.min = result.max = glm::dot(axis, vertex[0]);

		for (int i = 1; i < 8; ++i)
		{
			float projection = glm::dot(axis, vertex[i]);
			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}
	bool TDSAT::OverlapOnAxis(TDBox * obb1, TDBox * obb2, const glm::vec3 & axis, float& value)
	{
		Interval a = GetInterval(obb1, axis);
		Interval b = GetInterval(obb2, axis);
		value = a.min - b.min;
		return ((b.min <= a.max) && (a.min <= b.max));
	}
	Interval TDSAT::GetInterval(const TDAABB* aabb, const glm::vec3& axis)
	{
		glm::vec3 i = aabb->GetMin();
		glm::vec3 a = aabb->GetMax();

		glm::vec3 vertex[8] = {
			glm::vec3(i.x, a.y, a.z),
			glm::vec3(i.x, a.y, i.z),
			glm::vec3(i.x, i.y, a.z),
			glm::vec3(i.x, i.y, i.z),
			glm::vec3(a.x, a.y, a.z),
			glm::vec3(a.x, a.y, i.z),
			glm::vec3(a.x, i.y, a.z),
			glm::vec3(a.x, i.y, i.z)
		};

		Interval result;
		result.min = result.max = glm::dot(axis, vertex[0]);

		for (int i = 1; i < 8; ++i)
		{
			float projection = glm::dot(axis, vertex[i]);
			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}
	bool TDSAT::OverlapOnAxis(TDAABB * AABB, TDBox * obb2, const glm::vec3 & axis)
	{
		Interval a = GetInterval(AABB, axis);
		Interval b = GetInterval(obb2, axis);
		return ((b.min <= a.max) && (a.min <= b.max));
	}

};