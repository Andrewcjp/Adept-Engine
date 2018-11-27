#pragma once
#define RUN_NAN_CHECK !BUILD_SHIPPING
#if RUN_NAN_CHECK
#define CheckNAN(target) MathUtils::CheckNAN_(target);
#else
#define CheckNAN(target)
#endif
namespace MathUtils
{
	bool AlmostEqual(float a, float b, float epsilon);
	void CheckNAN_(glm::vec3 value);
	const int MAX_int = std::numeric_limits<int>::max();
	const float FloatMAX = std::numeric_limits<float>::max();
};
