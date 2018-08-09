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
 


