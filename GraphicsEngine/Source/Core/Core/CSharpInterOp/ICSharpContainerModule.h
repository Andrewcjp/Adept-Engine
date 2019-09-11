#pragma once
#include "..\Module\ModuleInterface.h"
class  ICSharpContainerModule: public IModuleInterface
{
public:
	CORE_API ICSharpContainerModule();
	CORE_API ~ICSharpContainerModule();
	CORE_API virtual void RunTest();
	CORE_API virtual void StartUp();
	CORE_API virtual void Tick();
	CORE_API virtual void ShutDown();
};

