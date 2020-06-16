#pragma once
#include "Core/Transform.h"
#include "Rendering/Core/Camera.h"

#include "Rendering/Core/Light.h"
#include "ShaderProgramBase.h"
#include "Core/Assets/ShaderCompiler.h"
//this is a basis for a shader 
//shaders will derive from this class so that the correct uniforms can be updated

//Static Object adds a Function ptr for the constructor to the shader Compiler
//Declare Permutation 
#define DECLARE_GLOBAL_SHADER(Type)\
static Shader* ConstructCompiledInstance_##Type(const ShaderInit & Data)\
{\
	return new Type(Data.Context);\
}\
NAME_SHADER(Type)

#define DECLARE_GLOBAL_SHADER_ARGS(Type,Datatype)\
static Shader* ConstructCompiledInstance_##Type(const ShaderInit & Data)\
{\
	return new Type(Data.Context,*((Datatype*)Data.Data));\
}\

#define IMPLEMENT_GLOBAL_SHADER(Type)\
DECLARE_GLOBAL_SHADER_PERMIUTATION(Type,Type,void*,0,nullptr)

#define CreateFunc(Type) Type::ConstructCompiledInstance_##Type

#define DECLARE_GLOBAL_SHADER_PERMIUTATION(Name,Type,Datatype,DataValue,ShouldCompileFunc)\
Datatype  _Type_##Name = DataValue; \
/*static*/ ShaderType Type_##Name = ShaderType(std::string(#Type) + std::to_string(DataValue),&CreateFunc(Type) , ShaderInit(&_Type_##Name, sizeof(Datatype)),ShouldCompileFunc); \

#define NAME_SHADER(Type) const std::string GetName() override{return #Type;}
//Helper Defines:
#define IMPLEMENT_GLOBAL_SHADER_RT(Type)\
DECLARE_GLOBAL_SHADER_PERMIUTATION(Type,Type,void*,0,&Shader_RTBase::IsShaderSupported)

#define IMPLEMENT_GLOBAL_SHADER_SM6(Type)\
DECLARE_GLOBAL_SHADER_PERMIUTATION(Type,Type,void*,0,&Shader_RTBase::IsShaderSupported_SM6)

#define DEFINE_CACHESHADERPARAM(var) static int var;
#define IMP_CACHESHADERPARAM(clas,var) int clas::var = -1;
#define CACHEPARM(var,name) var = GetSlotForName(#name);
#define CACHED_SHADER_PARM(var,name)

#define REGISTER_SHADER(name,file,stage) \
static SingleShaderCompile SHADER_##name = SingleShaderCompile(#name,file,stage);

#define REGISTER_SHADER_CS(name,file) REGISTER_SHADER(name,file,EShaderType::SHADER_COMPUTE);
#define REGISTER_SHADER_PS(name,file) REGISTER_SHADER(name,file,EShaderType::SHADER_FRAGMENT);
#define REGISTER_SHADER_VS(name,file) REGISTER_SHADER(name,file,EShaderType::SHADER_VERTEX);


#define REGISTER_SHADER_ONEARG(name,file,define,stage) \
static SingleShaderCompile SHADER_##name = SingleShaderCompile(#name,file,define,stage);

#define REGISTER_SHADER_CS_ONEARG(name,file,define) REGISTER_SHADER_ONEARG(name,file,define,EShaderType::SHADER_COMPUTE);
#define REGISTER_SHADER_PS_ONEARG(name,file,define) REGISTER_SHADER_ONEARG(name,file,define,EShaderType::SHADER_FRAGMENT);
#define REGISTER_SHADER_VS_ONEARG(name,file,define) REGISTER_SHADER_ONEARG(name,file,define,EShaderType::SHADER_VERTEX);


class Shader
{
public:	
	Shader();
	Shader(DeviceContext* context);
	virtual ~Shader();
	RHI_API ShaderProgramBase* GetShaderProgram();
	virtual const std::string GetName();
	virtual std::vector<ShaderParameter> GetShaderParameters();
	virtual std::vector<VertexElementDESC> GetVertexFormat();
	virtual bool IsComputeShader();
	virtual void ApplyToCommandList(RHICommandList* list);
	ShaderParameter * FindParam(const std::string & name);
	bool ChangeParamType(const std::string & name, ShaderParamType::Type type);
	RHI_API int GetSlotForName(const std::string & name);
	uint64 GetNameHash();
	static bool IsShaderSupported_SM6(const ShaderCompileSettings& args);
	
protected:
	virtual void CacheParms();
	ShaderProgramBase * m_Shader = nullptr;
	class DeviceContext* Device = nullptr;
	long Hash = 0;
};

const int ALBEDOMAP = 0;
const int SHADOWCUBEMAP = 10;
const int SHADOWCUBEMAP2 = 11;
const int SHADOWDIRMAP1 = 9;
const int NORMALMAP = 6;
const int DISPMAP = 5;