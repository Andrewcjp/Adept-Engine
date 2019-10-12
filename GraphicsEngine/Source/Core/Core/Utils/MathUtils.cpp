#include "MathUtils.h"

namespace MathUtils
{

	bool AlmostEqual(float a, float b, float epsilon)
	{
		return fabs(a - b) <= epsilon;
	}

	bool AlmostEqual(glm::vec3 a, glm::vec3 b, float epsilon)
	{
		const float SqEpsi = epsilon * epsilon;
		const float distanceSQ = glm::distance2(a, b);

		return distanceSQ <= SqEpsi;
	}

	void MathUtils::CheckNAN_(glm::vec3 value)
	{
		glm::vec3::bool_type t = glm::isnan(value);
		if (t.x == true || t.y == true || t.z == true)
		{
			DEBUGBREAK;
		}
	}

	void MathUtils::CheckNAN_(glm::quat value)
	{
		glm::vec4::bool_type t = glm::isnan(value);
		if (t.x == true || t.y == true || t.z == true || t.w == true)
		{
			DEBUGBREAK;
		}
	}

};
