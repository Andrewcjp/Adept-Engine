#pragma once
#define ENGINE_NAME "Adept Engine"

namespace Version
{
	long GetVersionNumber();
	int GetMajorRevision();
	int GetMinorRevision();
	int GetPatchRevision();
	CORE_API std::string GetVersionString();
	CORE_API std::string GetFullVersionString();
	CORE_API std::string GetBuildVersion();
	CORE_API int GetBuildNumber();
}