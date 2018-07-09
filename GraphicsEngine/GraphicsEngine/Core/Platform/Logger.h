#pragma once
#include <sstream>
#include <ios>
class Log
{
public:
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
};

