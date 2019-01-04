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
	bool AlmostEqual(glm::vec3 a, glm::vec3 b, float epsilon);
	void CheckNAN_(glm::vec3 value);
	void CheckNAN_(glm::quat value);
	const int MAX_int = std::numeric_limits<int>::max();
	const float FloatMAX = std::numeric_limits<float>::max();
	template<class T>
	T SafeNormalise(T type)
	{
		if (glm::length(type) != 0.0f)
		{
			return glm::normalize(type);
		}
		return type;
	}
	glm::vec3 VectorProject(const glm::vec3 length, const glm::vec3 direction);
};
