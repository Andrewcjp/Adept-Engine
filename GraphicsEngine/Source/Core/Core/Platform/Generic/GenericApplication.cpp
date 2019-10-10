
#include "GenericApplication.h"
#include "Core/Utils/StringUtil.h"
#include <experimental/filesystem>
#include "Core/Utils/FileUtils.h"
namespace fs = std::experimental::filesystem;
int GenericApplication::ExecuteHostScript(std::string Filename, std::string Args, bool ShowOutput)
{
	return 0;
}

int GenericApplication::ExecuteHostScript(std::string Filename, std::string Args, std::string WorkingDir, bool ShowOutput)
{
	return 0;
}

void GenericApplication::InitTiming()
{}

double GenericApplication::Seconds()
{
	return 0.0;
}

void GenericApplication::Sleep(float Milliseconds)
{}

int64_t GenericApplication::GetFileTimeStamp(const std::string& Path)
{
	return fs::_Last_write_time(StringUtils::ConvertStringToWide(Path).c_str());
}

bool GenericApplication::CheckFileSrcNewer(const std::string& srcpath, const std::string dest)
{
	if (!FileUtils::File_ExistsTest(srcpath) || !FileUtils::File_ExistsTest(dest))
	{
		return false;
	}
	int64_t time = GetFileTimeStamp(srcpath);
	int64_t CSOtime = GetFileTimeStamp(dest);
	return  !(time > CSOtime);
}

bool GenericApplication::TryCreateDirectory(const std::string & name)
{
	return false;
}

bool GenericApplication::IsDebuggerPresent()
{
	return false;
}

void GenericApplication::DisplayMessageBox(std::string title, std::string message)
{

}

bool GenericApplication::DisplayOpenFileDialog(std::string StartDir, std::string Filter, std::string & outData)
{
	return false;
}

bool GenericApplication::DisplaySaveFileDialog(std::string StartDir, std::string Filter, std::string Extenstion, std::string & outData)
{
	return false;
}

bool GenericApplication::CopyFileToTarget(std::string Target, std::string Dest)
{
	return false;
}

void GenericApplication::Init()
{}

void * GenericApplication::GetDllExport(void * DllHandle, const char * ProcName)
{
	return nullptr;
}

void GenericApplication::FreeDllHandle(void * DllHandle)
{}

void * GenericApplication::GetDllHandle(FString Name)
{
	return nullptr;
}
