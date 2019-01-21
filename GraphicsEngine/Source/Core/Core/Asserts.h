#pragma once
#include "Core/Engine.h"
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#if !BUILD_SHIPPING
#define DebugEnsure(condition) if(!condition){ __debugbreak();}
#define AssertDebugBreak() __debugbreak();
#else
#define DebugEnsure(condition);
#define AssertDebugBreak();
#endif
#if DOFULLCHECK
#define ensure(condition) {if(!(condition)){ AssertDebugBreak();std::string Message = "Ensure Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition ;\
Log::LogMessage(Message,Log::Severity::Error); PlatformApplication::DisplayMessageBox("Fatal Error", Message); Engine::RequestExit(-1);}}

#define ensureMsgf(condition,Message) ensureFatalMsgf(condition,Message);

#define check(condition) if(!condition){AssertDebugBreak();std::string Message = "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition ;\
Log::LogMessage(Message,Log::Severity::Error); PlatformApplication::DisplayMessageBox("Error",Message);}

#define checkMsgf(condition,Message) if(!condition){AssertDebugBreak();std::string data = "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition  "\n" Message ;\
Log::LogMessage(data,Log::Severity::Error); PlatformApplication::DisplayMessageBox("Error",data);}

#define NoImpl(){AssertDebugBreak(); PlatformApplication::DisplayMessageBox("Error", "Feature Not Implmented \n" __FILE__ "@" LINE_STRING);}
#else
#define check(condition);
#define checkMsgf(condition,Message);
#define ensure(condition);
#define ensureMsgf(condition,Message);
#define NoImpl();
#endif
#if DOCHECK
#define ensureFatalMsgf(condition,Message) if(!(condition)){ AssertDebugBreak();std::string data = "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition  "\n" Message ; \
Log::LogMessage(data,Log::Severity::Error); PlatformApplication::DisplayMessageBox("Fatal Error",data); Engine::RequestExit(-1);}
#else
#define ensureFatalMsgf(condition,Message);
#endif
