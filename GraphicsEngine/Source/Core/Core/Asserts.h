#pragma once

//#include "Core/Platform/PlatformCore.h"
#if DOCHECK
#define DebugEnsure(condition) if(!condition){ __debugbreak();}
#define ensure(condition) {if(!(condition)){ __debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Ensure Failed \n" #condition); exit(1359);}}
#define ensureMsgf(condition,Message) if(!(condition)){ __debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Ensure Failed \n" Message); exit(1359);}
#define check(condition) if(!condition){__debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Assert Failed \n" #condition);}
#define checkMsgf(condition,Message) if(!condition){__debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Assert Failed \n" Message);}
#define NoImpl(){__debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Feature Not Implmented \n");}
#else
#define DebugEnsure(condition);
#define check(condition);
#define checkMsgf(condition,Message);
#define ensure(condition);
#define ensureMsgf(condition,Message);
#define NoImpl();
#endif