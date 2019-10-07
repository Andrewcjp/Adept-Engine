#include "Stdafx.h"
//this file defines entry point redirectors to the engine DLL
#ifdef PLATFORM_WINDOWS
#include "WinLauncher.h"
#include <windows.h>
int  __declspec(dllimport) WinMain_Engine(HINSTANCE hInstance,	HINSTANCE,	LPSTR    lpCmdLine,	int       nCmdShow);

int WINAPI WinMain(
	HINSTANCE hInstance,     /* [input] handle to current instance */
	HINSTANCE hPrevInstance, /* [input] handle to previous instance */
	LPSTR lpCmdLine,         /* [input] pointer to command line */
	int nCmdShow             /* [input] show state of window */
)
{
	WinMain_Engine(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#endif