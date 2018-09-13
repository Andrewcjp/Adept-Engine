#pragma once
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#if !BUILD_SHIPPING && !BUILD_PACKAGE 
#define DebugEnsure(condition) if(!condition){ __debugbreak();}
#define AssertDebugBreak() __debugbreak();
#else
#define DebugEnsure(condition);
#define AssertDebugBreak()
#endif
#if DOFULLCHECK
#define ensure(condition) {if(!(condition)){ AssertDebugBreak(); PlatformApplication::DisplayMessageBox("Fatal Error", "Ensure Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition); exit(1359);}}
#define ensureMsgf(condition,Message) ensureFatalMsgf(condition,Message);
#define check(condition) if(!condition){AssertDebugBreak(); PlatformApplication::DisplayMessageBox("Error", "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition);}
#define checkMsgf(condition,Message) if(!condition){AssertDebugBreak(); PlatformApplication::DisplayMessageBox("Error", "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" Message);}
#define NoImpl(){AssertDebugBreak(); PlatformApplication::DisplayMessageBox("Error", "Feature Not Implmented \n" __FILE__ "@" LINE_STRING);}
#else
#define check(condition);
#define checkMsgf(condition,Message);
#define ensure(condition);
#define ensureMsgf(condition,Message);
#define NoImpl();
#endif
#if DOCHECK
#define ensureFatalMsgf(condition,Message) if(!(condition)){ AssertDebugBreak(); PlatformApplication::DisplayMessageBox("Fatal Error", "Ensure Failed \n " __FILE__ "@" LINE_STRING "\ncondition: " #condition "\n" Message);\
 exit(1359);}
#else
#define ensureFatalMsgf(condition,Message);
#endif
