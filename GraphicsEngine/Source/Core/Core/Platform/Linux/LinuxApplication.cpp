#include "LinuxApplication.h"
#ifdef PLATFORM_LINUX
EPlatforms::Type LinuxApplication::GetPlatform()
{
	return EPlatforms::Linux;
}
#endif