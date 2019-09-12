#include "Stdafx.h"
#include "CSharpContainer.h"
#include "ICSharpContainerModule.h"
#include "..\Module\ModuleManager.h"
CSharpContainer* CSharpContainer::Instance = nullptr;

CSharpContainer::CSharpContainer()
{}


CSharpContainer::~CSharpContainer()
{}

void CSharpContainer::StartUp()
{
	Instance = new CSharpContainer();
	Get()->CSContainer = ModuleManager::Get()->GetModule<ICSharpContainerModule>("CSharpContainer");
	Get()->CSContainer->StartUp();
	CSObjectCreationArgs Args;
	Args.ClassId = 23454;
	Get()->CSContainer->CreateObject(&Args);
}

void CSharpContainer::ShutDown()
{
	Get()->CSContainer->ShutDown();
	SafeDelete(Get()->CSContainer);
}

void CSharpContainer::Update(float DT)
{
	Get()->CSContainer->Tick();
}

CSharpContainer * CSharpContainer::Get()
{
	return Instance;
}


