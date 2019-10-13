#include "AndroidApplication.h"
#ifdef PLATFORM_ANDROID
EPlatforms::Type AndroidApplication::GetPlatform()
{
	return EPlatforms::Android;
}
#endif