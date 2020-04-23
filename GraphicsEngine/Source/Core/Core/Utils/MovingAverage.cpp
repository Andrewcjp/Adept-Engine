#include "MovingAverage.h"
#include "Core/Platform/PlatformCore.h"

MovingAverage::MovingAverage(unsigned short filterLength)
{
	FilterLength = filterLength;
	init();
}

MovingAverage::MovingAverage()
{
	FilterLength = default_filter_length;
	init();
}

MovingAverage::~MovingAverage()
{
	delete[] Array;
}

void MovingAverage::init()
{
	FilterComplete = false;
	Index = -1;
	Sum = 0;
	Average = 0;
	Array = new float[FilterLength];
	clear();
}

void MovingAverage::clear()
{
	for (unsigned short i = 0; i < FilterLength; i++)
	{
		Array[i] = 0;
	}
	Sum = 0;
	Average = 0.0f;
	Index = -1;
}

void MovingAverage::Add(float x)
{
	Index = (Index + 1) % FilterLength;
	Sum -= Array[Index];
	Array[Index] = x;
	Sum += x;
	if (!FilterComplete && Index == (FilterLength - 1))
	{
		FilterComplete = true;
	}
	if (FilterComplete)
	{
		Average = Sum / FilterLength;
	}
	else
	{
		Average = Sum / (Index + 1);
	}
	Sum = glm::max(Sum, 0.0f);
	Average = glm::max(Average, 0.0f);
	//ensure(Sum >= 0.0f);
}

float MovingAverage::GetCurrentAverage()
{
	return Average;
}

float* MovingAverage::GetArray()
{
	return Array;
}

float MovingAverage::GetRaw()
{
	if (Index == -1)
	{
		return 0.0f;
	}
	return Array[Index];
}

float MovingAverage::GetHighestValue()
{
	float Max = -1.0f;
	for (int i = 0; i < FilterLength; i++)
	{
		Max = Math::Max(Max, Array[i]);
	}
	return Max;
}

unsigned short MovingAverage::GetFilterLength()
{
	return FilterLength;
}

bool MovingAverage::IsLastIndex() const
{
	return Index == FilterLength - 1;
}
