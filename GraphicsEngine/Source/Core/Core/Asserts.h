#pragma once
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#if DOFULLCHECK
#define DebugEnsure(condition) if(!condition){ __debugbreak();}
#define ensure(condition) {if(!(condition)){ __debugbreak(); PlatformApplication::DisplayMessageBox("Fatal Error", "Ensure Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition); exit(1359);}}
#define ensureMsgf(condition,Message) ensureFatalMsgf(condition,Message);
#define check(condition) if(!condition){__debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition);}
#define checkMsgf(condition,Message) if(!condition){__debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" Message);}
#define NoImpl(){__debugbreak(); PlatformApplication::DisplayMessageBox("Error", "Feature Not Implmented \n" __FILE__ "@" LINE_STRING);}
#else
#define DebugEnsure(condition);
#define check(condition);
#define checkMsgf(condition,Message);
#define ensure(condition);
#define ensureMsgf(condition,Message);
#define NoImpl();
#endif
#if DOCHECK
#define ensureFatalMsgf(condition,Message) if(!(condition)){ __debugbreak(); PlatformApplication::DisplayMessageBox("Fatal Error", "Ensure Failed \n " __FILE__ "@" LINE_STRING "\ncondition: " #condition "\n" Message);\
 exit(1359);}
#else
#define ensureFatalMsgf(condition,Message);
#endif
