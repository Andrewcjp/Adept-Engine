#ifndef MOVING_AVERAGE__H
#define MOVING_AVERAGE__H

class CORE_API MovingAverage
{
public:
	MovingAverage(unsigned short length);
	MovingAverage();
	~MovingAverage();

	void Add(float value);
	unsigned short GetFilterLength();
	float GetCurrentAverage();
	void clear();
	float* GetArray();
private:
	const static unsigned short default_filter_length = 5;
	// Length of the filter
	unsigned short FilterLength;
	float * Array;
	float Sum;
	float Average;
	int Index;
	bool FilterComplete;
	void init();
};
#endif
