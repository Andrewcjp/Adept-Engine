#include "Source/Core/Stdafx.h"
#include "IOHandler.h"
#include <iosfwd>
#include "Logger.h"
#include <fstream>

bool IOHandler::LoadTextFile(std::string path, std::vector<std::string>& lines)
{
	std::ifstream myfile(path);
	if (myfile.is_open())
	{
		std::string line;
		while (std::getline(myfile, line))
		{
			lines.push_back(line);
		}
		myfile.close();
	}
	else
	{
		Log::LogMessage("Failed to load " + path, Log::Error);
		return false;
	}
	return true;
}
