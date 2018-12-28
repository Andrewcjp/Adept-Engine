#pragma once

namespace TD { class TDTriangle; }

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

		//Overlap Axis
		static bool OverlapOnAxis(const TDAABB * aabb, const TDTriangle * triangle, const glm::vec3 & axis);
		static bool OverlapOnAxis(TDBox * obb, const TDTriangle * triangle, const glm::vec3 & axis);
		static bool OverlapOnAxis(TDBox * obb1, TDBox * obb2, const glm::vec3 & axis, float& value);
		static bool OverlapOnAxis(TDAABB* AABB, TDBox * obb2, const glm::vec3 & axis);
		//Intervals 
		static Interval GetInterval(const TDAABB * aabb, const glm::vec3 & axis);
		static Interval GetInterval(TDBox * obb, const glm::vec3 & axis);
		static Interval GetInterval(const TDTriangle * triangle, const glm::vec3 & axis);
	};
};