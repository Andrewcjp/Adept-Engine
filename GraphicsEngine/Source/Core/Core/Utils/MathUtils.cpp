#include "MathUtils.h"

namespace MathUtils
{

	bool AlmostEqual(float a, float b, float epsilon)
	{
		return fabs(a - b) <= epsilon;
	}

	void MathUtils::CheckNAN_(glm::vec3 value)
	{
		glm::vec3::bool_type t = glm::isnan(value);
		if (t.x == true || t.y == true || t.z == true)
		{
			__debugbreak();
		}
	}

	void MathUtils::CheckNAN_(glm::quat value)
	{
		glm::vec4::bool_type t = glm::isnan(value);
		if (t.x == true || t.y == true || t.z == true || t.w == true)
		{
			__debugbreak();
		}
	}

};
