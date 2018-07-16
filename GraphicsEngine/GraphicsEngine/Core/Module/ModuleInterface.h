#pragma once

class IModuleInterface
{

public:
	virtual ~IModuleInterface()
	{}
	virtual void StartupModule()
	{}
	virtual void ShutdownModule()
	{}
};

#define IMPLEMENT_MODULE( ModuleImplClass) \
extern "C" DLLEXPORT IModuleInterface* InitializeModule() \
{ \
	return new ModuleImplClass(); \
} 
