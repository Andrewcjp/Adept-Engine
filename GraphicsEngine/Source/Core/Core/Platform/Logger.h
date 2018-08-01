#pragma once
#include <sstream>
#include <ios>
#include "EngineGlobals.h"
class CORE_API Log
{
public:
	enum Severity
	{
		Message,
		Warning,
		Error
	};
	static void LogOutput(std::string data);
	struct StreamWrapper
	{
		std::stringstream Out;
		template<class T>
		StreamWrapper& operator<<(const T& dt)
		{
			Out << dt;
			return *this;
		}
		StreamWrapper& operator<<(const StreamWrapper& dt)
		{
			Out << "\n";
			Log::LogOutput(Out.str());
			Out.str("");
			return *this;
		}
	};
	static StreamWrapper OutS;
	static void LogMessage(std::string msg, Severity s = Severity::Message);
};

