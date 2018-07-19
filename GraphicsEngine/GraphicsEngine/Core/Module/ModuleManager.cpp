#include "Stdafx.h"
#include "ModuleManager.h"
#include "../Core/Platform/PlatformCore.h"
ModuleManager* ModuleManager::Instance = nullptr;
ModuleManager * ModuleManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ModuleManager();		
	}
	return Instance;
}


ModuleManager::ModuleManager()
{}


ModuleManager::~ModuleManager()
{}

void ModuleManager::ShutDown()
{
	//Unload in reverse Module Load order
	for (int i = ModulesNames.size() - 1; i >= 0; i--)
	{
		UnloadModule(ModulesNames[i]);
	}
}


void ModuleManager::SetupPreLoadModules()
{
	ModulesNames.push_back("TestGame");
}

void ModuleManager::PreLoadModules()
{
	SetupPreLoadModules();
	for (int i = 0; i < ModulesNames.size(); i++)
	{
		LoadModule(ModulesNames[i]);
	}
}

void ModuleManager::UnloadModule(FString name)
{
	if (Modules.find(name.ToSString()) != Modules.end())
	{
		ModuleInfo* Info = &Modules.at(name.ToSString());
		if (Info->ModuleStatus != ModuleLoadStatus::Status_Loaded)
		{
			return;
		}
		Info->Module->ShutdownModule();
		//The interface has no data in it!
		Info->Module = nullptr;
		PlatformApplication::FreeDllHandle(Info->Handle);
		Info->ModuleStatus = ModuleLoadStatus::Status_UnLoaded;
	}
}

IModuleInterface * ModuleManager::LoadModule(FString Name)
{
	Modules.emplace(Name.ToSString(), ModuleInfo());
	ModuleInfo* Info = &Modules.at(Name.ToSString());
	if (Info->IsDynamic)
	{
		Info->Handle = PlatformApplication::GetDllHandle(Name);
		if (Info->Handle == nullptr)
		{
			Info->ModuleStatus = ModuleLoadStatus::Status_LoadFailed;
			return nullptr;
		}
		FInitializeModuleFunctionPtr ModuleInitPtr = (FInitializeModuleFunctionPtr)PlatformApplication::GetDllExport(Info->Handle, "InitializeModule");
		Info->Module = (IModuleInterface*)ModuleInitPtr();
	}
	else // Module is Static!
	{
		if (StaticModulePtrs.find(Name.ToSString()) != StaticModulePtrs.end())
		{
			FInitializeModuleFunctionPtr ModuleInitPtr = StaticModulePtrs.at(Name.ToSString());
			Info->Module = (IModuleInterface*)ModuleInitPtr();

		}
	}
	if (Info->Module == nullptr)
	{
		Info->ModuleStatus = ModuleLoadStatus::Status_LoadFailed;
		return nullptr;
	}
	Info->Module->StartupModule();
	Info->ModuleStatus = ModuleLoadStatus::Status_Loaded;
	return Info->Module;
}




