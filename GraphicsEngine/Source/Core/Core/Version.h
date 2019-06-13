#pragma once
#define BUILD_STRING "1.0.3 06-01-19"
#define ENGINE_NAME "Adept Engine"

namespace Version
{
	long GetVersionNumber();
	int GetMajorRevision();
	int GetMinorRevision();
	int GetPatchRevision();
	std::string GetVersionString();
	std::string GetFullVersionString();
	std::string GetBuildVersion();
}