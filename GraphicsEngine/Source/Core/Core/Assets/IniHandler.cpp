#include "IniHandler.h"
#include "AssetManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Platform/IOHandler.h"
#include "Core/Utils/FileUtils.h"

IniHandler::IniHandler()
{
	MainCFGfile = AssetManager::GetSettingsDir() + "\\CFG.ini";
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
	if (!IOHandler::LoadTextFile(MainCFGfile, lines))
	{
		return;
	}
	ConsoleVariableManager::Get()->SetupCVarsFromCFG(lines);
}
