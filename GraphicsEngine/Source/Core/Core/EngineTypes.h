#pragma once
#include "Core/Types/FString.h"
#include "Utils/MemoryUtils.h"
typedef glm::vec3 Colour;
namespace Colours
{
	static Colour RED = Colour(1, 0, 0);
};

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
	IntRect(int width, int height) :
		IntRect(IntPoint(0, 0), IntPoint(width, height))
	{}
	IntRect(IntPoint min, IntPoint max)
	{
		Min = min;
		Max = max;
	}
	IntPoint Dim;
};

struct FloatPoint
{
	float x = 0;
	float y = 0;
	FloatPoint()
	{}
	FloatPoint(float X, float Y)
	{
		x = X;
		y = Y;
	}
};

struct FloatRect
{
	FloatPoint Min;
	FloatPoint Max;
	FloatRect()
	{}
	FloatRect(FloatPoint min, FloatPoint max)
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
	CORE_API void SetFlagValue(int flag, bool value);
	CORE_API bool GetFlagValue(int flag) const;
	CORE_API void SetFlags(int flags);
	CORE_API static int ConvertToFlags(int enumi);
	CORE_API int GetFlags() { return Flags; }
private:
	int Flags = 0;
};

typedef BitFlagsBase SceneLayerMask;

typedef BitFlagsBase GPUDeviceMask;

namespace ESceneLayers
{
	enum Type
	{
		Default = 1,
		UserLayer0,
		UserLayer1,
		Limit = 16
	};
};
template<class T>
class CachedAllocator
{
public:
	~CachedAllocator()
	{
		Empty();
	}
	//new data
	T* Allocate()
	{
		if (AllocIndex < AllocBuffer.size())
		{
			AllocIndex++;
			return AllocBuffer[AllocIndex - 1];
		}
		T* NewData = new T();
		AllocBuffer.push_back(NewData);
		AllocIndex++;
		return NewData;
	}
	//resets the alloc for new use
	void Reset()
	{
		AllocIndex = 0;
	}
	void Empty()
	{
		MemoryUtils::DeleteVector<T>(AllocBuffer);
		AllocIndex = 0;
	}
private:
	std::vector<T*> AllocBuffer;
	int AllocIndex = 0;
};
