#include "FileUtils.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Platform/PlatformCore.h"
#include <fstream>

bool FileUtils::File_ExistsTest(const std::string & name, bool Silent)
{
	struct stat buffer;
	if ((stat(name.c_str(), &buffer) == 0))
	{
		return true;
	}
	if (!Silent)
	{
		Log::OutS << "File Does not exist " << name.c_str() << Log::OutS;
	}
	return false;
}

bool FileUtils::CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot)
{
	if (!RelativeToRoot)
	{
		StringUtils::RemoveChar(Path, root);
	}
	if (FileUtils::File_ExistsTest(root + Path))
	{
		return true;
	}
	std::vector<std::string> split = StringUtils::Split(Path, '\\');
	std::string FirstPath = root;
	for (int i = 0; i < split.size(); i++)
	{
		FirstPath += "\\" + split[i];
		if (!FileUtils::File_ExistsTest(FirstPath))
		{
			if (!PlatformApplication::TryCreateDirectory(FirstPath))
			{
				return false;
			}
		}
	}
	return false;
}

bool FileUtils::CreateDirectoriesToFullPath(std::string Path)
{
	if (FileUtils::File_ExistsTest(Path))
	{
		return true;
	}
	std::vector<std::string> split = StringUtils::Split(Path, '\\');
	std::string FirstPath = "";
	for (int i = 0; i < split.size(); i++)
	{
		if (split[i].find(".") != -1)
		{
			continue;
		}
		if (i == 0)
		{
			FirstPath = split[i];
		}
		else
		{
			FirstPath += "\\" + split[i];
		}
		if (!FileUtils::File_ExistsTest(FirstPath))
		{
			if (!PlatformApplication::TryCreateDirectory(FirstPath))
			{
				return false;
			}
		}
	}
	return false;
}

bool FileUtils::WriteToFile(std::string Filename, std::string data, bool append)
{
	std::string out;
	char flags = std::ofstream::out;
	if (append)
	{
		flags |= std::fstream::app;
	}
	std::ofstream myfile(Filename, flags);
	if (!myfile.is_open())
	{
		Log::OutS << "Failed to Write " << Filename << Log::OutS;
		return false;
	}
	std::string line;
	myfile.write(data.c_str(), data.length());
	myfile.close();

	return true;
}

FileUtils::CSVWriter::CSVWriter(std::string filename)
{
	FileTarget = filename;
}

void FileUtils::CSVWriter::Save()
{
	if (FileTarget.empty())
	{
		return;
	}
	WriteToFile(FileTarget, FileContents);
}

void FileUtils::CSVWriter::AddEntry(std::string Data)
{
	FileContents.append(Data);
	FileContents.append(", ");
}

void FileUtils::CSVWriter::AddLineBreak()
{
	FileContents.append("\r");
}

void FileUtils::CSVWriter::Clear()
{
	FileContents = "";
}
