#pragma once
#include <functional>
#include "Rendering\Core\Mesh\MaterialTypes.h"
#include "RHI\DeviceContext.h"
#include "RHI\ShaderProgramBase.h"
//Two types of shaders present:
//1) class based complied and handled though their class type
//2) source based - complied and handled though a hash.
class Shader_NodeGraph;
class ShaderGraphCompiler;
class IShaderCompiler;
struct ShaderCompileItem;
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
struct ShaderCompileSettings
{
	bool RTSupported = false;
	EShaderSupportModel::Type ShaderModel = EShaderSupportModel::SM5;
};
struct ShaderType
{
	typedef std::function<class Shader*(const ShaderInit &)> InitliserFunc;
	typedef std::function<bool(const ShaderCompileSettings& args)> ShouldComplieSig;
	ShaderType(std::string name, InitliserFunc constructor, const ShaderInit & init, ShouldComplieSig ShouldComplieFunc);
	InitliserFunc Constructor;
	ShaderInit ShaderInitalizer;
	Shader* CompiledShader = nullptr;
	ShouldComplieSig ShouldCompileFunc;
};
struct SingleShaderCompile
{
	std::string Name;
	SingleShaderCompile(std::string name, std::string file, ShaderProgramBase::Shader_Define Define, EShaderType::Type stage);
	SingleShaderCompile(std::string name, std::string file, EShaderType::Type stage);
	std::string SourceFile;
	std::vector<ShaderProgramBase::Shader_Define> Defines;
	EShaderType::Type Stage = EShaderType::SHADER_UNDEFINED;
	uint64 GetHash();
};
struct MaterialShaderPair
{
	Shader_NodeGraph* Placeholder;
	MaterialShaderCompileData Data;
};
class ShaderCompiler
{
public:
	ShaderCompiler();
	~ShaderCompiler();
	RHI_API static ShaderCompiler* Get();
	RHI_API static void ShutDown();
	void CompileAllGlobalShaders();
	void FreeAllGlobalShaders();
	RHI_API bool ShouldBuildDebugShaders();
	void CompileShader(ShaderType & type, DeviceContext* Context);
	RHI_API ShaderType * GetShaderFromGlobalMap(std::string name, DeviceContext* context = nullptr);
	void AddShaderType(std::string Name, ShaderType  type);
	void TickMaterialCompile();
	template<class T>
	static T* GetShader()
	{
		//default is listed as "0"
		ShaderType* CachedShader = ShaderCompiler::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::string("0"));
		if (CachedShader != nullptr)
		{
			if (CachedShader->CompiledShader == nullptr)
			{
				CachedShader->CompiledShader = new T(RHI::GetDefaultDevice());
			}
			return (T*)(CachedShader->CompiledShader);
		}
		DebugEnsure(false);
		return nullptr;
	}
	template<class T>
	static T* GetShader(class DeviceContext* dev)
	{
		ShaderType* CachedShader = ShaderCompiler::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::string("0"), dev);
		if (CachedShader != nullptr)
		{
			if (CachedShader->CompiledShader == nullptr)
			{
				CachedShader->CompiledShader = new T(dev);
			}
			return (T*)(CachedShader->CompiledShader);
		}
		return nullptr;
	}
	template<class T, class U>
	static T* GetShader(class DeviceContext* dev, U Data)
	{
		ShaderType* CachedShader = ShaderCompiler::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::to_string(Data), dev);
		if (CachedShader != nullptr)
		{
			if (CachedShader->CompiledShader == nullptr)
			{
				CachedShader->CompiledShader = new T(dev, Data);
			}
			return (T*)(CachedShader->CompiledShader);
		}
		return nullptr;
	}
	template<class T, class U>
	static T* GetShader_Default(U Data)
	{
		return GetShader<T>(RHI::GetDefaultDevice(), Data);
	}
	Shader_NodeGraph* GetMaterialShader(MaterialShaderCompileData Data);
	Shader_NodeGraph* CompileMateral(MaterialShaderCompileData data);
	void CompileMaterialShader(Shader_NodeGraph * shader);
	//returns a place holder object which will be populated later
	Shader_NodeGraph* EnqeueueMaterialShadercompile(MaterialShaderCompileData data);
	void RegisterShaderCompiler(std::string DLLName);;
	void Init();
	void CompileShaderNew(ShaderCompileItem * shader, EPlatforms::Type platform = EPlatforms::Windows);
	void RegisterSingleShader(SingleShaderCompile* Target);
	bool CheckSourceFileRegistered(std::string file);
	struct ShaderCompilerConfig
	{
		EPlatforms::Type TargetPlatform = EPlatforms::Windows;
		bool MirrorToOthers = true;
		EShaderSupportModel::Type ShaderModelTarget = EShaderSupportModel::SM6;
	};
	ShaderCompilerConfig m_Config;
private:
	std::vector<std::string> ShaderCompilerNames;
	void FindAndLoadCompilers();
	std::vector<IShaderCompiler*> ShaderCompilers;
	ShaderGraphCompiler* MaterialCompiler = nullptr;
	static ShaderCompiler * Instance;
	typedef std::map<std::string, ShaderType> ShaderMap;
	ShaderMap GlobalShaderMapDefinitions;
	ShaderMap GlobalShaderMap[MAX_GPU_DEVICE_COUNT];
	ShaderMap* GetShaderMap(DeviceContext* device = nullptr);
	std::map<std::string, Shader_NodeGraph*> MaterialShaderMap;
	std::queue<MaterialShaderPair> MaterialShaderCompileQueue;
	bool ComplieShadersOnTheFly = false;
	std::map<uint64, SingleShaderCompile*> SingleShaderMapDefinitions;
	std::map<uint64, SingleShaderCompile*> SingleShaderMap[MAX_GPU_DEVICE_COUNT];
};
