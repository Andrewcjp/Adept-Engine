#pragma once
#include "Core/Types/FString.h"
#include "ModuleInterface.h"
#include <vector>
#include <map>
#include "Core/Platform/Logger.h"
#include <functional>
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
typedef std::function<IModuleInterface*()> FStaticInitializeModuleFunctionPtr;
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
	std::map<std::string, FStaticInitializeModuleFunctionPtr> StaticModulePtrs;
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
		bool IsDynamic = false;
#endif
		
	};
	std::map<std::string, ModuleInfo> Modules;

};



template<class ModuleClass>
class StaticModuleReg
{
public:
	StaticModuleReg(const char* name)
	{
		//__debugbreak();
		//std::bind(&EditorWindow::EnterPlayMode, EditorWindow::GetInstance()
		FStaticInitializeModuleFunctionPtr Funcptr = std::bind(&StaticModuleReg::StaticInitializeModule);
		//ensure(false);
		ModuleManager::Get()->StaticModulePtrs.emplace(name, Funcptr);
	}

	static IModuleInterface* StaticInitializeModule()
	{
		return new ModuleClass();
	}
};
#define IMPLEMENT_MODULE_STATIC( ModuleImplClass,ModName) static StaticModuleReg< ModuleImplClass > ModuleRegistrant##ModuleImplClass( #ModName ); 

#define IMPLEMENT_MODULE_DYNAMIC( ModuleImplClass) \
extern "C" DLLEXPORT IModuleInterface* InitializeModule() \
{ \
	return new ModuleImplClass(); \
} 

#if 0
//Module Test
class TestMode :public IModuleInterface
{
	
};
IMPLEMENT_MODULE_STATIC(TestMode);
#endif