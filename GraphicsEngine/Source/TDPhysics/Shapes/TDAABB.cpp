#include "TDPCH.h"
#include "TDAABB.h"
namespace TD
{

	TDAABB::TDAABB()
	{

	}


	TDAABB::~TDAABB()
	{

	}
	glm::vec3 TDAABB::GetMin()
	{
		glm::vec3 point1 = Position + HalfExtends;
		glm::vec3 point2 = Position - HalfExtends;
		return glm::vec3(fminf(point1.x, point2.x),	fminf(point1.y, point2.y),fminf(point1.z, point2.z));
	}
	glm::vec3 TDAABB::GetMax()
	{
		glm::vec3 point1 = Position + HalfExtends;
		glm::vec3 point2 = Position - HalfExtends;
		return glm::vec3(fmaxf(point1.x, point2.x),	fmaxf(point1.y, point2.y), fmaxf(point1.z, point2.z));
	}
};