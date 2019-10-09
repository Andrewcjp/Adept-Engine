#pragma once
#ifdef PLATFORM_ANDROID
#include "..\Generic\GenericPlatformMisc.h"
class AndroidPlatformMisc : public GenericPlatformMisc
{

};

typedef AndroidPlatformMisc PlatformMisc;
#endif
