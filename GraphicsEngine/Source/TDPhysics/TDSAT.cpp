#include "TDSAT.h"
#include "Shapes/TDAABB.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDMeshShape.h"
namespace TD
{
	TDSAT::SatInterval TDSAT::GetInterval(TDBox* box, const glm::vec3 axis)
	{
		const int VertCount = 8;
		glm::vec3 vertex[VertCount];

		const glm::vec3 Centre = box->GetPos();
		const glm::vec3 HalfExtends = box->HalfExtends;
		glm::mat3x3 Rotation = glm::mat3(box->GetTransfrom()->GetQuatRot());
		glm::vec3 u0 = glm::vec3(Rotation[0][0], Rotation[1][0], Rotation[2][0]);
		glm::vec3 u1 = glm::vec3(Rotation[0][1], Rotation[1][1], Rotation[2][1]);//todo: wrong way round?
		glm::vec3 u2 = glm::vec3(Rotation[0][2], Rotation[1][2], Rotation[2][2]);

		vertex[0] = Centre + Rotation[0] * HalfExtends[0] + Rotation[1] * HalfExtends[1] + Rotation[2] * HalfExtends[2];
		vertex[1] = Centre - Rotation[0] * HalfExtends[0] + Rotation[1] * HalfExtends[1] + Rotation[2] * HalfExtends[2];
		vertex[2] = Centre + Rotation[0] * HalfExtends[0] - Rotation[1] * HalfExtends[1] + Rotation[2] * HalfExtends[2];
		vertex[3] = Centre + Rotation[0] * HalfExtends[0] + Rotation[1] * HalfExtends[1] - Rotation[2] * HalfExtends[2];
		vertex[4] = Centre - Rotation[0] * HalfExtends[0] - Rotation[1] * HalfExtends[1] - Rotation[2] * HalfExtends[2];
		vertex[5] = Centre + Rotation[0] * HalfExtends[0] - Rotation[1] * HalfExtends[1] - Rotation[2] * HalfExtends[2];
		vertex[6] = Centre - Rotation[0] * HalfExtends[0] + Rotation[1] * HalfExtends[1] - Rotation[2] * HalfExtends[2];
		vertex[7] = Centre - Rotation[0] * HalfExtends[0] - Rotation[1] * HalfExtends[1] + Rotation[2] * HalfExtends[2];

		SatInterval result;
		result.min = result.max = glm::dot(axis, vertex[0]);

		for (int i = 1; i < VertCount; ++i)
		{
			float projection = glm::dot(axis, vertex[i]);
			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}

	bool TDSAT::OverlapOnAxis(TDBox * BoxA, TDBox * BoxB, glm::vec3 axis, float& value)
	{
		SatInterval a = GetInterval(BoxA, axis);
		SatInterval b = GetInterval(BoxB, axis);
		value = a.min - b.min;
		return ((b.min <= a.max) && (a.min <= b.max));
	}

	TDSAT::SatInterval TDSAT::GetInterval(const TDAABB* aabb, const glm::vec3 axis)
	{
		glm::vec3 min = aabb->GetMin();
		glm::vec3 Max = aabb->GetMax();
		const int VertCount = 8;
		glm::vec3 Verts[VertCount] = {
			glm::vec3(min.x, Max.y, Max.z),
			glm::vec3(min.x, Max.y, min.z),
			glm::vec3(min.x, min.y, Max.z),
			glm::vec3(min.x, min.y, min.z),
			glm::vec3(Max.x, Max.y, Max.z),
			glm::vec3(Max.x, Max.y, min.z),
			glm::vec3(Max.x, min.y, Max.z),
			glm::vec3(Max.x, min.y, min.z)
		};

		SatInterval result;
		result.min = result.max = glm::dot(axis, Verts[0]);

		for (int i = 1; i < VertCount; ++i)
		{
			float projection = glm::dot(axis, Verts[i]);
			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}

	bool TDSAT::OverlapOnAxis(TDAABB * AABB, TDBox * Box, const glm::vec3 axis)
	{
		SatInterval a = GetInterval(AABB, axis);
		SatInterval b = GetInterval(Box, axis);
		return ((b.min <= a.max) && (a.min <= b.max));
	}

	TDSAT::SatInterval TDSAT::GetInterval(const TDTriangle* triangle, const glm::vec3 axis)
	{
		SatInterval result;
		result.min = glm::dot(axis, triangle->Points[0]);
		result.max = result.min;
		const int TrangleSides = 3;
		for (int i = 1; i < TrangleSides; ++i)
		{
			float value = glm::dot(axis, triangle->Points[i]);
			result.min = fminf(result.min, value);
			result.max = glm::max(result.max, value);
		}

		return result;
	}

	bool TDSAT::OverlapOnAxis(const TDAABB* aabb, const TDTriangle* triangle, const glm::vec3 axis)
	{
		const SatInterval a = GetInterval(aabb, axis);
		const SatInterval b = GetInterval(triangle, axis);
		return (b.min <= a.max) && (a.min <= b.max);
	}

	bool TDSAT::OverlapOnAxis(TDBox* obb, const TDTriangle* triangle, const glm::vec3 axis)
	{
		const SatInterval a = GetInterval(obb, axis);
		const SatInterval b = GetInterval(triangle, axis);
		return (b.min <= a.max) && (a.min <= b.max);
	}
};