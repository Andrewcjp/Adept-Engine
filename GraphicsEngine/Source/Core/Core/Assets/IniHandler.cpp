#include "Source/Core/Stdafx.h"
#include "IniHandler.h"
#include "AssetManager.h"
#include "Core/Platform/IOHandler.h"
#include "../Platform/ConsoleVariable.h"
#include "../Performance/BenchMarker.h"
#include "../Utils/FileUtils.h"

IniHandler::IniHandler()
{
	MainCFGfile = AssetManager::GetSettingsDir() + "\\test.ini";
}


IniHandler::~IniHandler()
{}

void IniHandler::SaveAllConfigProps()
{
	std::vector <std::string> lines;
	ConsoleVariableManager::Get()->GetCFGVariables(lines);
	std::string LineOut = "";
	for (int i = 0; i < lines.size(); i++)
	{
		LineOut.append(lines[i] + "\n");
	}
	FileUtils::WriteToFile(MainCFGfile, LineOut);
}

void IniHandler::LoadMainCFG()
{
	std::vector <std::string> lines;
	IOHandler::LoadTextFile(MainCFGfile, lines);
	ConsoleVariableManager::Get()->SetupCVarsFromCFG(lines);
}
