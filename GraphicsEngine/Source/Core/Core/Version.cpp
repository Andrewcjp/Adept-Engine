#include "Version.h"
#include "../Core/GitCommitVersion.h"
// Major release version
const int Major = 0;
// New features added, large Updates
const int Minor = 1;
// minor fixes, update, performance tweaks etc
const int Revision = 40;

long Version::GetVersionNumber()
{
	return Major * 10000000 + Minor * 10000 + Revision;
}

int Version::GetMajorRevision()
{
	return Major;
}

int Version::GetMinorRevision()
{
	return Minor;
}

int Version::GetPatchRevision()
{
	return Revision;
}

std::string Version::GetVersionString()
{
	return std::to_string(Major) + "." + std::to_string(Minor) + "." + std::to_string(Revision); 
}

std::string Version::GetFullVersionString()
{
	return std::to_string(Major) + "." + std::to_string(Minor) + "." + std::to_string(Revision) + " (Build: " + GetBuildVersion()+")";
}

std::string Version::GetBuildVersion()
{
	return std::to_string(GITCOMMIT);
}