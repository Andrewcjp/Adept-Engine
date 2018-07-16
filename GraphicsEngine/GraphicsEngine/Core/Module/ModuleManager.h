#pragma once
#include "../Core/Types/FString.h"
#include "ModuleInterface.h"
#include <vector>
#include <map>
#include "../Core/Platform/Logger.h"
namespace ModuleLoadStatus
{
	enum Type
	{
		Status_UnLoaded,
		Status_Loaded,
		Status_LoadFailed
	};
};
class ModuleManager
{
public:
	static ModuleManager* Get();
	ModuleManager();
	~ModuleManager();
	void ShutDown();
	void * GetDllHandle(LPCWSTR Name);
	template<class T>
	T* GetModule(FString Name)
	{
		std::string Data = Name.ToSString();
		if (Modules.find(Data) != Modules.end())
		{
			ModuleInfo* Info = &Modules.at(Data);
			if (Info->ModuleStatus != ModuleLoadStatus::Status_Loaded)
			{
				Log::OutS << "ERROR: Failed to Load Module " << Data << Log::OutS;
				return nullptr;
			}
			return (T*)Info->Module;
		}
		return (T*)LoadModule(Name);
	}
private:
	std::vector<FString> ModulesNames;
	void SetupPreLoadModules();
	void PreLoadModules();
	void UnloadModule(FString name);	
	static ModuleManager* Instance;
	IModuleInterface * LoadModule(FString Name);
	void * GetDllExport(void * DllHandle, const CHAR * ProcName);
	void FreeDllHandle(void * DllHandle);
	struct ModuleInfo
	{
		void* Handle = nullptr;
		FString Name;
		IModuleInterface* Module = nullptr;
		ModuleLoadStatus::Type ModuleStatus = ModuleLoadStatus::Status_UnLoaded;
	};
	std::map<std::string, ModuleInfo> Modules;
	ModuleInfo TestModule;
};

typedef IModuleInterface* (*FInitializeModuleFunctionPtr)(void);
