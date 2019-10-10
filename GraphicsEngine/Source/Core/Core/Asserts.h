#pragma once
#include "Core/Engine.h"
#include "Core\Platform\PlatformCore.h"
#ifdef PLATFORM_WINDOWS
#define DEBUGBREAK  __debugbreak();
#else
#define DEBUGBREAK
#endif
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#if !BUILD_SHIPPING
#define DebugEnsure(condition) if(!condition){DEBUGBREAK}
#define AssertDebugBreak() if(PlatformApplication::IsDebuggerPresent()){DEBUGBREAK} 
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

#define LogEnsure(condition) if(!(condition)){ /*AssertDebugBreak();*/std::string Message = "Ensure Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition ;\
Log::LogMessage(Message,Log::Severity::Error);}

#define LogEnsureMsgf(condition,Message) if(!(condition)){std::string data = "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition  "\n" Message ;\
Log::LogMessage(data,Log::Severity::Error);}

#define ENUMCONVERTFAIL() ensureMsgf(false,"Failed to convert Enum")
#else
#define check(condition);
#define checkMsgf(condition,Message);
#define ensure(condition);
#define ensureMsgf(condition,Message);
#define NoImpl();
#define LogEnsure(condition);
#define LogEnsureMsgf(c,m);
#define ENUMCONVERTFAIL()
#endif

#if DOCHECK
#define ensureFatalMsgf(condition,Message) if(!(condition)){ AssertDebugBreak();std::string data = "Assert Failed \n" __FILE__ "@" LINE_STRING"\ncondition:" #condition  "\n" Message ; \
Log::LogMessage(data,Log::Severity::Error); PlatformApplication::DisplayMessageBox("Fatal Error",data); Engine::RequestExit(-1);}
#else
#define ensureFatalMsgf(condition,Message);
#endif


