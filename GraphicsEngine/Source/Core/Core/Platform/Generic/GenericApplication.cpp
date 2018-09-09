#include "Stdafx.h"
#include "GenericApplication.h"
#include "Core/Utils/StringUtil.h"
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
int64_t GenericApplication::GetFileTimeStamp(const std::string Path)
{
	return fs::_Last_write_time(StringUtils::ConvertStringToWide(Path).c_str());
}
