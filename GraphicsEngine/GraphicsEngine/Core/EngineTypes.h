#pragma once
#include "glm/glm.hpp"
typedef  glm::vec3 Colour;
namespace Colours
{
	static Colour RED = Colour(1, 0, 0);
}
struct IntPoint
{
	int x = 0;
	int y = 0;
	IntPoint() {}
	IntPoint(int X, int Y)
	{
		x = X;
		y = Y;
	}
};
struct IntRect
{
	IntPoint Min;
	IntPoint Max;
	IntRect() {}
	IntRect(IntPoint min, IntPoint max)
	{
		Min = min;
		Max = max;
	}
};