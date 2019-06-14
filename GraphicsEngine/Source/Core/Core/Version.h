#pragma once
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
	int GetBuildNumber();
}