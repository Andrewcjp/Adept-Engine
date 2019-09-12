#include "CSharpBridgeModule.h"
#include "Core\Module\ModuleManager.h"
#include "CSharpBridge\ManagedBridge.h"

CSharpBridgeModule::CSharpBridgeModule()
{}


CSharpBridgeModule::~CSharpBridgeModule()
{}

#ifdef CSHARPBRIDGE_EXPORT
IMPLEMENT_MODULE_DYNAMIC(CSharpBridgeModule);
#endif