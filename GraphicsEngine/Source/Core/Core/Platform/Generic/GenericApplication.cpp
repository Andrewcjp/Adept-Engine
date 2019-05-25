
#include "GenericApplication.h"
#include "Core/Utils/StringUtil.h"
#include <experimental/filesystem>
#include "Core/Utils/FileUtils.h"
namespace fs = std::experimental::filesystem;
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