#include "EngineVersion.h"
#include "Core/Version.h"
using namespace System::Runtime::InteropServices;


int ManagedDll::Container::CallCreateObject(CSObjectCreationArgs* Args)
{
	//System::IntPtr buffer = Marshal::AllocCoTaskMem(Marshal::SizeOf(CSObjectCreationArgs));
	//CSharpCore::CSObjectCreationArgs data;
	//Marshal::StructureToPtr(data, buffer, false);
	//		
	CSharpCore::CSObjectCreationArgs data;
	data.ClassId = Args->ClassId;
	//System::IntPtr ptr = (IntPtr)Args;
	//
	//Marshal::PtrToStructure(ptr, data);
	return CSharpCore::Container::CreateObject(data);
}
