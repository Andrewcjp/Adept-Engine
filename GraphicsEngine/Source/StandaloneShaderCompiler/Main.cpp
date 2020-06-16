#include "Stdafx.h"
//this file defines entry point redirectors to the engine DLL
#ifdef PLATFORM_WINDOWS
#include "RHI/StandaloneShaderCompilerEntry.h"
int main(int argc, char *argv[])
{
	StandaloneShaderCompilerEntry Entry(argc, argv);
	return Entry.Execute();
}
#endif
