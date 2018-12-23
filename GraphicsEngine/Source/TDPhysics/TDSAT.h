#pragma once

namespace TD
{
	class TDBox;
	class TDAABB;
	struct Interval
	{
		float min;
		float max;
	};
	class TDSAT
	{
	public:

		static Interval GetInterval(TDBox * obb, const glm::vec3 & axis);
		static bool OverlapOnAxis(TDBox * obb1, TDBox * obb2, const glm::vec3 & axis, float& value);
		static Interval GetInterval(const TDAABB * aabb, const glm::vec3 & axis);
		static bool OverlapOnAxis(TDAABB* AABB, TDBox * obb2, const glm::vec3 & axis);
	};
};