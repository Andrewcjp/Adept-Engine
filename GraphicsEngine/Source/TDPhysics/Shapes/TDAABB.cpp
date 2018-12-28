#include "TDAABB.h"
#include "TDPhysics.h"
#include "TDSimConfig.h"
namespace TD
{
	TDAABB::TDAABB()
	{
		ShapeType = TDShapeType::eAABB;
	}

	TDAABB::TDAABB(glm::vec3 Pos, glm::vec3 size) :TDAABB()
	{
		Position = Pos;
		HalfExtends = size;
	}

	TDAABB::~TDAABB()
	{}

	glm::vec3 TDAABB::GetMin() const
	{
		glm::vec3 point1 = Position + HalfExtends;
		glm::vec3 point2 = Position - HalfExtends;
		return glm::vec3(fminf(point1.x, point2.x), fminf(point1.y, point2.y), fminf(point1.z, point2.z));
	}

	glm::vec3 TDAABB::GetMax() const
	{
		glm::vec3 point1 = Position + HalfExtends;
		glm::vec3 point2 = Position - HalfExtends;
		return glm::vec3(fmaxf(point1.x, point2.x), fmaxf(point1.y, point2.y), fmaxf(point1.z, point2.z));
	}

	void TDAABB::DebugRender(glm::vec3 colour /*= glm::vec3(1)*/, float time /*= 0.0f*/)
	{
		if (!TDPhysics::GetCurrentSimConfig()->ShowBroadPhaseShapes)
		{
			return;
		}
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, -HalfExtends.y, HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, HalfExtends.y, HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, HalfExtends.y, HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, HalfExtends.y, -HalfExtends.z), Position + glm::vec3(HalfExtends.x, -HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, -HalfExtends.y, -HalfExtends.z), Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, -HalfExtends.z), Position + glm::vec3(-HalfExtends.x, HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, HalfExtends.y, -HalfExtends.z), Position + glm::vec3(HalfExtends.x, HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, -HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, -HalfExtends.z), colour, time);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, HalfExtends.y, -HalfExtends.z), colour, time);
	}
	glm::vec3 TDAABB::ClosestPoint(const glm::vec3& point)const
	{
		glm::vec3 result = point;
		glm::vec3 min = GetMin();
		glm::vec3 max = GetMax();

		result.x = (result.x < min.x) ? min.x : result.x;
		result.y = (result.y < min.y) ? min.y : result.y;
		result.z = (result.z < min.z) ? min.z : result.z;

		result.x = (result.x > max.x) ? max.x : result.x;
		result.y = (result.y > max.y) ? max.y : result.y;
		result.z = (result.z > max.z) ? max.z : result.z;

		return result;
	}
	TDAABB* TDAABB::CreateFromMinMax(const glm::vec3 & min, const glm::vec3 & max)
	{
		TDAABB* r = new TDAABB();
		r->HalfExtends = (max - min) * 0.5f;
		r->Position = (min + max) * 0.5f;
		return r;
	}
};