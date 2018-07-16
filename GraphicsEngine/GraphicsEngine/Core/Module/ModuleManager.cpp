#include "Stdafx.h"
#include "ModuleManager.h"
ModuleManager* ModuleManager::Instance = nullptr;
ModuleManager * ModuleManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ModuleManager();
		Instance->PreLoadModules();
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
	for (int i = ModulesNames.size()-1; i >= 0 ; i--)
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
		FreeDllHandle(Info->Handle);
		Info->ModuleStatus = ModuleLoadStatus::Status_UnLoaded;
	}
}

IModuleInterface * ModuleManager::LoadModule(FString Name)
{
	Modules.emplace(Name.ToSString(), ModuleInfo());
	ModuleInfo* Info = &Modules.at(Name.ToSString());
	Info->Handle = GetDllHandle(Name.ToWideString().c_str());
	if (Info->Handle == nullptr)
	{
		Info->ModuleStatus = ModuleLoadStatus::Status_LoadFailed;
		return nullptr;
	}
	FInitializeModuleFunctionPtr ModuleInitPtr = (FInitializeModuleFunctionPtr)GetDllExport(Info->Handle, "InitializeModule");
	Info->Module = (IModuleInterface*)ModuleInitPtr();
	if (Info->Module == nullptr)
	{
		Info->ModuleStatus = ModuleLoadStatus::Status_LoadFailed;
		return nullptr;
	}
	Info->Module->StartupModule();
	Info->ModuleStatus = ModuleLoadStatus::Status_Loaded;
	return Info->Module;
}

//temp funcs
void* ModuleManager::GetDllExport(void* DllHandle, const CHAR* ProcName)
{
	/*check(DllHandle);
	check(ProcName);*/
	return (void*)::GetProcAddress((HMODULE)DllHandle, (ProcName));
}

void ModuleManager::FreeDllHandle(void* DllHandle)
{
	// It is okay to call FreeLibrary on 0
	::FreeLibrary((HMODULE)DllHandle);
}


void* ModuleManager::GetDllHandle(LPCWSTR Name)
{
	void * Handle = LoadLibrary(Name);
	if (!Handle)
	{
		//log!
	}
	return Handle;
}
