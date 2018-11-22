#include "TDAABB.h"
#include "TDPhysics.h"
namespace TD
{
	TDAABB::TDAABB()
	{}

	TDAABB::~TDAABB()
	{}

	glm::vec3 TDAABB::GetMin()
	{
		glm::vec3 point1 = Position + HalfExtends;
		glm::vec3 point2 = Position - HalfExtends;
		return glm::vec3(fminf(point1.x, point2.x), fminf(point1.y, point2.y), fminf(point1.z, point2.z));
	}

	glm::vec3 TDAABB::GetMax()
	{
		glm::vec3 point1 = Position + HalfExtends;
		glm::vec3 point2 = Position - HalfExtends;
		return glm::vec3(fmaxf(point1.x, point2.x), fmaxf(point1.y, point2.y), fmaxf(point1.z, point2.z));
	}

	void TDAABB::DebugRender()
	{
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, -HalfExtends.y, HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, HalfExtends.y, HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, HalfExtends.y, HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, HalfExtends.y, -HalfExtends.z), Position + glm::vec3(HalfExtends.x, -HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, -HalfExtends.y, -HalfExtends.z), Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, -HalfExtends.z), Position + glm::vec3(-HalfExtends.x, HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, HalfExtends.y, -HalfExtends.z), Position + glm::vec3(HalfExtends.x, HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(HalfExtends.x, -HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, -HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
		TDPhysics::DrawDebugLine(Position + glm::vec3(-HalfExtends.x, HalfExtends.y, HalfExtends.z), Position + glm::vec3(-HalfExtends.x, HalfExtends.y, -HalfExtends.z), glm::vec3(1), 0.0f);
	}
};