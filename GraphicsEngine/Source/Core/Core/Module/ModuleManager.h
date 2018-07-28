#pragma once
#include "Core/Types/FString.h"
#include "ModuleInterface.h"
#include <vector>
#include <map>
#include "Core/Platform/Logger.h"
namespace ModuleLoadStatus
{
	enum Type
	{
		Status_UnLoaded,
		Status_Loaded,
		Status_LoadFailed
	};
};
typedef IModuleInterface* (*FInitializeModuleFunctionPtr)(void);
class ModuleManager
{
public:
	static ModuleManager* Get();
	ModuleManager();
	~ModuleManager();
	void ShutDown();
	
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
	std::map<std::string, FInitializeModuleFunctionPtr> StaticModulePtrs;
	void PreLoadModules();
private:
	std::vector<FString> ModulesNames;
	void SetupPreLoadModules();
	
	void UnloadModule(FString name);	
	static ModuleManager* Instance;
	IModuleInterface * LoadModule(FString Name);
	struct ModuleInfo
	{
		void* Handle = nullptr;
		FString Name;
		IModuleInterface* Module = nullptr;
		ModuleLoadStatus::Type ModuleStatus = ModuleLoadStatus::Status_UnLoaded;
#ifdef NDEBUG
		bool IsDynamic = true;
#else
		bool IsDynamic = true;
#endif
		
	};
	std::map<std::string, ModuleInfo> Modules;

};
#if 1
template<class ModuleClass>
class StaticModuleReg
{
public:
	StaticModuleReg(const char* name)
	{
		//__debugbreak();
		ModuleManager::Get()->StaticModulePtrs.emplace(name, &StaticInitializeModule());
	}
	IModuleInterface* StaticInitializeModule()
	{
		return new ModuleClass();
	}
};
#define IMPLEMENT_MODULE_STATIC( ModuleImplClass) static FStaticallyLinkedModuleRegistrant< ModuleImplClass > ModuleRegistrant##ModuleImplClass( #ModuleImplClass ); 

#endif