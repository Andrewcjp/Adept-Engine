#pragma once
#include <string>
#include "../Core/Asserts.h"

class GenericApplication
{
public:
	bool ExecuteHostScript(std::string Filename, std::string Args) { /*ensureMsgf(false, "Generic Application Called")*/ };
	static void InitTiming() {};
	static double Seconds() { };
	static void Sleep(float Milliseconds) {  };
};

