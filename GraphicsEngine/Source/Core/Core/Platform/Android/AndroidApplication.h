#pragma once
#ifdef PLATFORM_ANDROID
#include "..\Generic\GenericApplication.h"
class AndroidApplication : public GenericApplication
{

};
typedef AndroidApplication PlatformApplication;
#endif