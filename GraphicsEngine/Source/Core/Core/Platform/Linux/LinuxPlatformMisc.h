#pragma once
#ifdef PLATFORM_LINUX
class LinuxPlatformMisc : public GenericPlatformMisc
{
};

typedef LinuxPlatformMisc PlatformMisc;
#endif
