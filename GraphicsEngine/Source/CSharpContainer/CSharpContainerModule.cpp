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

#ifdef CSHARPCONTAINER_EXPORT
IMPLEMENT_MODULE_DYNAMIC(CSharpContainerModule);
#endif