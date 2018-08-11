#pragma once

class IModuleInterface
{

public:
	virtual ~IModuleInterface()
	{}
	virtual bool StartupModule()
	{
		return true;
	}
	virtual void ShutdownModule()
	{}
};
 


