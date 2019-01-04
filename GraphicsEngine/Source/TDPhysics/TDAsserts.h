#pragma once
#ifdef TDPHYSICS_EXPORT
#if !BUILD_FULLRELEASE
#define DebugEnsure(condition) if(!(condition)){ __debugbreak();}
#define AssertDebugBreak() __debugbreak();
#else
#define DebugEnsure(condition);
#define AssertDebugBreak()
#endif
#endif