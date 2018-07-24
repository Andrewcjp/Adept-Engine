#include "Stdafx.h"
#include "GenericPlatformMisc.h"
#include <ctime> 

std::string GenericPlatformMisc::GetDateTimeString()
{
	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);
	std::stringstream out;
	out << (now->tm_year + 1900) << '-' << (now->tm_mon + 1) << '-' << now->tm_mday << "-" << (now->tm_hour) << "-" << (now->tm_min) << "-" << (now->tm_sec);
	return out.str();
}
