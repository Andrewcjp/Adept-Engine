#include "Logger.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Utils/FileUtils.h"
#include "PlatformCore.h"
#include "UI/UIManager.h"

Log::StreamWrapper Log::OutS;
Log* Log::Instance = nullptr;

CORE_API void Log::LogOutput(std::string data, int colour, bool ForceFlush /*= false*/)
{
	if (Instance == nullptr)
	{
		return;
	}
	Instance->LogData.push_back(data);
	if (ForceFlush || Instance->LogData.size() % 10)
	{
		Instance->FlushToLogFile();
	}
	PlatformMisc::LogPlatformOutput(data); 
	PlatformMisc::SetConsoleOutputColour(colour);
	printf(data.c_str());
}

void Log::LogBoolTerm(std::string PreText, bool value, int ForceOffset)
{
	LogOutput(PreText, 7);
	if (ForceOffset != -1)
	{
		const int NeededPad = ForceOffset - (int)PreText.size();
		if (NeededPad > 0)
		{
			std::string Pad = std::string(NeededPad, ' ');
			LogOutput(Pad, 7);
		}
	}
	LogOutput("[" + StringUtils::BoolToString(value) + "]", value ? 2 : 4);
	LogOutput("\n", 7);
}

void Log::LogMessage(std::string msg, Severity s)
{
	std::string data = "";
	int colour = 7;
	switch (s)
	{
	case Severity::Warning:
		data.append("Warning: ");
		colour = 6;
		break;
	case Severity::Error:
		data.append("Error: ");
		colour = 4;
		break;
	}
	data.append(msg);
	if (s == Severity::Progress)
	{
		data.append(std::string(msg.length(), '\b'));
	}
	else
	{
		data.append("\n");
	}
	LogOutput(data, colour, s == Severity::Error);
}

void Log::LogTextToScreen(std::string msg, float LifeTime /*= 0.0f*/, bool showinLog /*= false*/)
{
	if (Instance != nullptr)
	{
		ScreenLogEntry entry;
		entry.LifeTime = LifeTime;
		entry.Data = msg;
		Instance->ScreenLogLines.push_back(entry);
		if (showinLog)
		{
			LogMessage(msg, Log::Severity::Message);
		}
	}
}

void Log::StartLogger()
{
	Instance = new Log();
}

void Log::ShutDownLogger()
{
	Instance->FlushToLogFile();
	SafeDelete(Instance);
}

Log * Log::Get()
{
	return Instance;
}

void Log::RenderText(UIManager* Manager, int offset)
{
	if (Manager != nullptr)
	{
		for (int i = (int)ScreenLogLines.size() - 1; i >= 0; i--)
		{
			Manager->RenderTextToScreen(offset + i, ScreenLogLines[i].Data);
			ScreenLogLines[i].LifeTime -= Engine::GetDeltaTime();
			if (ScreenLogLines[i].LifeTime <= 0.0f)
			{
				ScreenLogLines.erase(ScreenLogLines.begin() + i);
			}
		}
	}
}

Log::Log()
{
	LogFilePath = AssetManager::DirectGetGeneratedDir() + "\\Log.txt";
	std::string data = "Log Started ";
	data.append(PlatformMisc::GetDateTimeString());
	data.append("\n");
	FileUtils::WriteToFile(LogFilePath, data);
}

void Log::FlushToLogFile()
{
	std::string outputData;
	for (int i = LastIndex; i < LogData.size(); i++)
	{
		outputData.append(LogData[i]);
	}
	LastIndex = (int)LogData.size();
	FileUtils::WriteToFile(LogFilePath, outputData, true);
}
