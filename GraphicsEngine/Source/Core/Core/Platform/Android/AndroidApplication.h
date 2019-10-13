#pragma once
#ifdef PLATFORM_ANDROID
#include "..\Generic\GenericApplication.h"
class AndroidApplication : public GenericApplication
{
	static EPlatforms::Type GetPlatform();
};
typedef AndroidApplication PlatformApplication;
#endif