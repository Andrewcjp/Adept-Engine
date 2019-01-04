#pragma once

#include <String.h>
#include <iostream>
namespace FileUtils
{
	CORE_API bool File_ExistsTest(const std::string& name, bool Silent = true);
	bool CreateDirectoryFromFullPath(std::string root, std::string Path, bool RelativeToRoot);
	CORE_API bool CreateDirectoriesToFullPath(std::string Path);
	CORE_API bool WriteToFile(std::string Filename, std::string data, bool append = false);
	class CSVWriter
	{
	public:
		CSVWriter(std::string filename);
		void Save();
		void AddEntry(std::string Data);
		void AddLineBreak();
		void Clear();
	private:
		std::string FileTarget = "";
		std::string FileContents = "";
	};
}