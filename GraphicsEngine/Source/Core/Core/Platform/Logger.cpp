
#include "Logger.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Utils/FileUtils.h"
#include "UI/UIManager.h"

Log::StreamWrapper Log::OutS;
Log* Log::Instance = nullptr;

void Log::LogOutput(std::string data, bool ForceFlush/* = false*/)
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
	printf(data.c_str());
}

void Log::LogMessage(std::string msg, Severity s)
{
	std::string data = "";
	switch (s)
	{
	case Severity::Message:
		//data.append("Message:");
		break;
	case Severity::Warning:
		data.append("Warning: ");
		break;
	case Severity::Error:
		data.append("Error: ");
		break;
	}
	data.append(msg);
	data.append("\n");
	LogOutput(data, s == Severity::Error);
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
		for (int i = ScreenLogLines.size() - 1; i >= 0; i--)
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
