#pragma once
#include <functional>
#include "Rendering\Core\Mesh\MaterialTypes.h"

class Shader_NodeGraph;
class ShaderGraphComplier;
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

struct ShaderType
{
	typedef std::function<class Shader*(const ShaderInit &)> InitliserFunc;
	ShaderType(std::string name, InitliserFunc constructor, ShaderInit & init);
	InitliserFunc Constructor;
	ShaderInit ShaderInitalizer;
	Shader* CompliedShader = nullptr;
};

class ShaderComplier
{
public:
	ShaderComplier();
	~ShaderComplier();
	RHI_API static ShaderComplier* Get();
	void ComplieAllGlobalShaders();
	void FreeAllGlobalShaders();
	RHI_API bool ShouldBuildDebugShaders();
	void ComplieShader(ShaderType & type, DeviceContext* Context);
	ShaderType * GetShaderFromGlobalMap(std::string name);
	void AddShaderType(std::string Name, ShaderType  type);
	void TickMaterialComplie();
	template<class T>
	static T* GetShader()
	{
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
		ShaderType* CachedShader = ShaderComplier::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::string("0"));
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
		ShaderType* CachedShader = ShaderComplier::Get()->GetShaderFromGlobalMap(typeid(T).name() + std::to_string(Data));
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
	void EnqeueueMaterialShadercomplie(MaterialShaderComplieData data);
private:
	ShaderGraphComplier* MaterialCompiler = nullptr;
	static ShaderComplier * Instance;
	std::map<std::string, ShaderType> GlobalShaderMap;
	std::map<std::string, Shader_NodeGraph*> MaterialShaderMap;
	std::queue<MaterialShaderComplieData> MaterialShaderComplieQueue;
	bool ComplieShadersOnTheFly = false;
};
