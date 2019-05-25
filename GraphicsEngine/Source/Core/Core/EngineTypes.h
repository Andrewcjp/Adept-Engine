#pragma once
#include "Core/Types/FString.h"
typedef glm::vec3 Colour;
namespace Colours
{
	static Colour RED = Colour(1, 0, 0);
}
struct IntPoint
{
	int x = 0;
	int y = 0;
	IntPoint()
	{}
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
	IntRect()
	{}
	IntRect(IntPoint min, IntPoint max)
	{
		Min = min;
		Max = max;
	}
};
struct Tag
{
	CORE_API Tag()
	{}
	Tag(std::string s)
	{
		name = s;
	}
	std::string name;
	bool operator==(const Tag& t);
};
struct TagContainer
{
	std::vector<Tag> Tags;
	CORE_API bool Contains(const Tag& t);
	CORE_API void Add(const std::string& name);
};

class BitFlagsBase
{
public:
	void SetFlagValue(int flag, bool value);
	bool GetFlagValue(int flag);
	void SetFlags(int flags);
private:
	int Flags = 0;
};