#pragma once
#include "Core\Module\ModuleInterface.h"
#include "Core\CSharpInterOp\ICSharpContainerModule.h"
#include "CSharpBridge\EngineVersion.h"
class CSharpContainerModule : public ICSharpContainerModule
{
public:
	CSharpContainerModule();
	~CSharpContainerModule();

	virtual void StartUp() override;
	virtual void Tick() override;
	virtual void ShutDown() override;

	virtual int CreateObject(CSObjectCreationArgs* Args) override;

private:
	
};

