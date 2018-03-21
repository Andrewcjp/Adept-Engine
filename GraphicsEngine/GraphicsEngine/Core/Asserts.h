#pragma once
#define DOCHECK 1
#include "Utils\WindowsHelper.h"
#if DOCHECK
#define DebugEnsure(condition) if(!condition){ __debugbreak();}
#define ensure(condition) if(!(condition)){ __debugbreak(); WindowsHelpers::DisplayMessageBox("Error", "Ensure Failed \n" #condition); exit(1359);}
#define check(condition) if(!condition){__debugbreak(); WindowsHelpers::DisplayMessageBox("Error", "Assert Failed \n" #condition);}
#else
#define DebugEnsure(condition);
#define check(condition);
#define ensure(condition);
#endif