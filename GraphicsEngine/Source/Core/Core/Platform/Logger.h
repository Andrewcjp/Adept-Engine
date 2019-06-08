#pragma once
#include <sstream>
#include <ios>

class UIManager;

struct ScreenLogEntry
{
	float LifeTime = 0.0f;
	std::string Data = "";
};

class Log
{
public:
	enum Severity
	{
		Message,
		Progress,
		Warning,
		Error
	};
	CORE_API static void LogOutput(const std::string& data, int colour, bool ForceFlush = false);
	static void LogBoolTerm(std::string PreText, bool value, int ForceOffset = -1);
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
			Log::LogOutput(Out.str(), 7);
			Out.str("");
			return *this;
		}
	};
	static StreamWrapper OutS;
	CORE_API static void LogMessage(std::string msg, Severity s = Severity::Message);
	CORE_API static void LogTextToScreen(const std::string& msg, float LifeTime = 0.0f, bool showinLog = false);
	static void StartLogger();
	static void ShutDownLogger();
	static Log* Get();
	void RenderText(UIManager* Manager, int offset);
private:
	Log();
	static Log* Instance;
	void FlushToLogFile();
	std::vector<std::string> LogData;
	std::vector<ScreenLogEntry> ScreenLogLines;
	std::string LogFilePath = "";
	int LastIndex = 0;
};

