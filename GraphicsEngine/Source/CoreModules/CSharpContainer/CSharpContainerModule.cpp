#include "CSharpContainerModule.h"
#include "Core\Module\ModuleManager.h"
#include "Core\BaseWindow.h"
#include "CSharpBridge\EngineVersion.h"


CSharpContainerModule::CSharpContainerModule()
{}


CSharpContainerModule::~CSharpContainerModule()
{}

void CSharpContainerModule::StartUp()
{
	ManagedDll::Container container;
	container.CallInit();
}

void CSharpContainerModule::Tick()
{
	ManagedDll::Container container;
	container.CallTick();
}

void CSharpContainerModule::ShutDown()
{
	ManagedDll::Container container;
	container.CallShutdown();
}

int CSharpContainerModule::CreateObject(CSObjectCreationArgs* Args)
{
	ManagedDll::Container container;
	return container.CallCreateObject(Args);
}

#ifdef CSHARPCONTAINER_EXPORT
IMPLEMENT_MODULE_DYNAMIC(CSharpContainerModule);
#endif