namespace TD
{
	class TDBox;
	class TDTriangle;
	class TDAABB;
	struct SatInterval
	{
		float min;
		float max;
	};
	class TDSAT
	{
	public:

		//Overlap Axis
		static bool OverlapOnAxis(const TDAABB * aabb, const TDTriangle * triangle, const glm::vec3  axis);
		static bool OverlapOnAxis(TDBox * box, const TDTriangle * triangle, const glm::vec3  axis);
		static bool OverlapOnAxis(TDBox * boxA, TDBox * BoxB, const glm::vec3 axis, float& value);
		static bool OverlapOnAxis(TDAABB* AABB, TDBox * BoxB, const glm::vec3 axis);
		//Intervals 
		static SatInterval GetInterval(const TDAABB * aabb, const glm::vec3 axis);
		static SatInterval GetInterval(TDBox * obb, const glm::vec3 axis);
		static SatInterval GetInterval(const TDTriangle * triangle, const glm::vec3 axis);
	};
};