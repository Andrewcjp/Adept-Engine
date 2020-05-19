#pragma once
struct ProcessorData
{
	uint PhysicalCores = 0;
	uint LogicalCores = 0;
};
class HardwareReporter
{
public:
	static void Report();
	static ProcessorData GetData();
};

