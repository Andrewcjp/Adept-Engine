#include "Stdafx.h"
//this file defines entry point redirectors to the engine DLL
#ifdef PLATFORM_WINDOWS
#include "RHI/StandaloneShaderComplierEntry.h"
int main(int argc, char *argv[])
{
	StandaloneShaderComplierEntry Entry(argc, argv);
	return Entry.Execute();
}
#endif
