#pragma once
#include <sstream>
#include <ios>

class Log
{
public:
	enum Severity
	{
		Message,
		Warning,
		Error
	};
	CORE_API static void LogOutput(std::string data, bool ForceFlush = false);
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
	CORE_API static void LogMessage(std::string msg, Severity s = Severity::Message);
	static void StartLogger();
	static void ShutDownLogger();
private:
	Log();
	static Log* Instance;
	void FlushToLogFile();
	std::vector<std::string> LogData;
	std::string LogFilePath = "";
	int LastIndex = 0;
};

