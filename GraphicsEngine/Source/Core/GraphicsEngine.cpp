#include "GraphicsEngine.h"
#include "Core/Engine.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include <strsafe.h>
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "d3dcompiler.lib")
#define USE_SEP_CONSOLE !BUILD_SHIPPING

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE,
	LPSTR    lpCmdLine,
	int       nCmdShow)
{

	int exitcode = 0;
#if USE_SEP_CONSOLE
	//Allocate a console window
	//so that messages can be redirected to stdout
	if (!AllocConsole())
	{
		ErrorExit(TEXT("AllocConsole"));
	}

	FILE* pf_out;
	freopen_s(&pf_out, "CONOUT$", "w", stdout);
#endif

	Engine* engine = new Engine();
	engine->ProcessCommandLineInput((const CHAR *)lpCmdLine, nCmdShow);
	engine->PreInit();
	if (!engine->GetIsCooking())
	{
		PlatformWindow* myapp = PlatformWindow::CreateApplication(engine, hInstance, lpCmdLine, nCmdShow);
		exitcode = myapp->Run();
		myapp->DestroyApplication();
	}
	engine->Destory();
	SafeDelete(engine);

#if USE_SEP_CONSOLE
	fclose(pf_out);
	//Free the console window
	if (!FreeConsole())
	{
		ErrorExit(TEXT("FreeConsole"));
	}
#endif
	return exitcode;
}
