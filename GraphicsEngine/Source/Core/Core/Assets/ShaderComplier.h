#pragma once
#include <functional>
#include "Rendering\Core\Mesh\MaterialTypes.h"
#include "RHI\DeviceContext.h"
#include "RHI\ShaderProgramBase.h"
//Two types of shaders present:
//1) class based complied and handled though their class type
//2) source based - complied and handled though a hash.
class Shader_NodeGraph;
class ShaderGraphComplier;
class IShaderComplier;
struct ShaderComplieItem;
struct ShaderInit
{
	ShaderInit()
	{}
	ShaderInit(void* Valueptr, int DataSize)
	{
		Data = malloc(DataSize);
		memcpy(Data, Valueptr, DataSize);
	}
	class DeviceContext* Context = nullptr;
	void* Data = nullptr;
};
struct ShaderComplieSettings
{
	bool RTSupported = false;
	EShaderSupportModel::Type ShaderModel = EShaderSupportModel::SM5;
};
struct ShaderType
{
	typedef std::function<class Shader*(const ShaderInit &)> InitliserFunc;
	typedef std::function<bool(const ShaderComplieSettings& args)> ShouldComplieSig;
	ShaderType(std::string name, InitliserFunc constructor, const ShaderInit & init, ShouldComplieSig ShouldComplieFunc);
	InitliserFunc Constructor;
	ShaderInit ShaderInitalizer;
	Shader* CompliedShader = nullptr;
	ShouldComplieSig ShouldComplieFunc;
};
struct SingleShaderComplie
{
	std::string Name;
	SingleShaderComplie(std::string name, std::string file, ShaderProgramBase::Shader_Define Define, EShaderType::Type stage);
	SingleShaderComplie(std::string name, std::string file, EShaderType::Type stage);
	std::string SourceFile;
	std::vector<ShaderProgramBase::Shader_Define> Defines;
	EShaderType::Type Stage = EShaderType::SHADER_UNDEFINED;
	uint64 GetHash();
};
struct MaterialShaderPair
{
	Shader_NodeGraph* Placeholder;
	MaterialShaderComplieData Data;
};
class ShaderComplier
{
public:
	ShaderComplier();
	~ShaderComplier();
	RHI_API static ShaderComplier* Get();
	RHI_API static void ShutDown();
	void ComplieAllGlobalShaders();
	void FreeAllGlobalShaders();
	RHI_API bool ShouldBuildDebugShaders();
	void ComplieShader(ShaderType & type, DeviceContext* Context);
	RHI_API ShaderType * GetShaderFromGlobalMap(std::string name, DeviceContext* context = nullptr);
	void AddShaderType(std::string Name, ShaderType  type);
	void TickMaterialComplie();
	template<class T>
	static T* GetShader()
	{
		//default is listed as "0"
		ShaderType* CachedShader = ShaderComplier::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::string("0"));
		if (CachedShader != nullptr)
		{
			if (CachedShader->CompliedShader == nullptr)
			{
				CachedShader->CompliedShader = new T(RHI::GetDefaultDevice());
			}
			return (T*)(CachedShader->CompliedShader);
		}
		DebugEnsure(false);
		return nullptr;
	}
	template<class T>
	static T* GetShader(class DeviceContext* dev)
	{
		ShaderType* CachedShader = ShaderComplier::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::string("0"), dev);
		if (CachedShader != nullptr)
		{
			if (CachedShader->CompliedShader == nullptr)
			{
				CachedShader->CompliedShader = new T(dev);
			}
			return (T*)(CachedShader->CompliedShader);
		}
		return nullptr;
	}
	template<class T, class U>
	static T* GetShader(class DeviceContext* dev, U Data)
	{
		ShaderType* CachedShader = ShaderComplier::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::to_string(Data), dev);
		if (CachedShader != nullptr)
		{
			if (CachedShader->CompliedShader == nullptr)
			{
				CachedShader->CompliedShader = new T(dev, Data);
			}
			return (T*)(CachedShader->CompliedShader);
		}
		return nullptr;
	}
	template<class T, class U>
	static T* GetShader_Default(U Data)
	{
		return GetShader<T>(RHI::GetDefaultDevice(), Data);
	}
	Shader_NodeGraph* GetMaterialShader(MaterialShaderComplieData Data);
	Shader_NodeGraph* ComplieMateral(MaterialShaderComplieData data);
	void ComplieMaterialShader(Shader_NodeGraph * shader);
	//returns a place holder object which will be populated later
	Shader_NodeGraph* EnqeueueMaterialShadercomplie(MaterialShaderComplieData data);
	void RegisterShaderComplier(std::string DLLName);;
	void Init();
	void ComplieShaderNew(ShaderComplieItem * shader, EPlatforms::Type platform = EPlatforms::Windows);
	void RegisterSingleShader(SingleShaderComplie* Target);
	bool CheckSourceFileRegistered(std::string file);
	struct ShaderComplierConfig
	{
		EPlatforms::Type TargetPlatform = EPlatforms::Windows;
		bool MirrorToOthers = true;
		EShaderSupportModel::Type ShaderModelTarget = EShaderSupportModel::SM6;
	};
	ShaderComplierConfig m_Config;
private:
	std::vector<std::string> ShaderComplierNames;
	void FindAndLoadCompliers();
	std::vector<IShaderComplier*> Shadercompliers;
	ShaderGraphComplier* MaterialCompiler = nullptr;
	static ShaderComplier * Instance;
	typedef std::map<std::string, ShaderType> ShaderMap;
	ShaderMap GlobalShaderMapDefinitions;
	ShaderMap GlobalShaderMap[MAX_GPU_DEVICE_COUNT];
	ShaderMap* GetShaderMap(DeviceContext* device = nullptr);
	std::map<std::string, Shader_NodeGraph*> MaterialShaderMap;
	std::queue<MaterialShaderPair> MaterialShaderComplieQueue;
	bool ComplieShadersOnTheFly = false;
	std::map<uint64, SingleShaderComplie*> SingleShaderMapDefinitions;
	std::map<uint64, SingleShaderComplie*> SingleShaderMap[MAX_GPU_DEVICE_COUNT];
};
