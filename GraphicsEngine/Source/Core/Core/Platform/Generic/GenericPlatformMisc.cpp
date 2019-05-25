
#include "GenericPlatformMisc.h"
#include <ctime> 

std::string GenericPlatformMisc::GetDateTimeString()
{
	time_t t = std::time(0);   // get time now
	tm* now = new tm();
	localtime_s(now, &t);
	std::stringstream out;
	out << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << "-" << (now->tm_hour) << "-" << (now->tm_min) << "-" << (now->tm_sec);
	delete now;
	return out.str();
}

float PlatformMemoryInfo::GetWorkingSetInMB()
{
	return (float)WorkingSetSize / 1024.0f / 1024.0f;
}
