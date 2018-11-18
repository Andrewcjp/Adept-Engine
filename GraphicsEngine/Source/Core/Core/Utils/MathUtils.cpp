#include "MathUtils.h"
namespace MathUtils
{
	bool AlmostEqual(float a, float b, float epsilon)
	{
		return fabs(a - b) <= epsilon;
	}
};
