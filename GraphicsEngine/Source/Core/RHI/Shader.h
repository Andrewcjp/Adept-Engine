#pragma once
#include "Core/Transform.h"
#include "Rendering/Core/Camera.h"

#include "Rendering/Core/Light.h"
#include "ShaderProgramBase.h"
#include "Core/Assets/ShaderComplier.h"
//this is a basis for a shader 
//shaders will derive from this class so that the correct uniforms can be updated

//Static Object adds a Function ptr for the constructor to the shader complier
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


#define DECLARE_GLOBAL_SHADER_PERMIUTATION(Name,Type,Datatype,DataValue,ShouldComplieFunc)\
Datatype  _Type_##Name = DataValue; \
/*static*/ ShaderType Type_##Name = ShaderType(std::string(#Type) + std::to_string(DataValue), &##Type::ConstructCompiledInstance_##Type, ShaderInit(&_Type_##Name, sizeof(Datatype)),ShouldComplieFunc); \

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
class Shader
{
public:

	typedef
		enum INPUT_CLASSIFICATION
	{
		INPUT_CLASSIFICATION_PER_VERTEX_DATA = 0,
		INPUT_CLASSIFICATION_PER_INSTANCE_DATA = 1
	} 	INPUT_CLASSIFICATION;
	typedef struct VertexElementDESC
	{
		char* SemanticName;
		unsigned int SemanticIndex;
		eTEXTURE_FORMAT Format;
		unsigned int InputSlot;
		unsigned int AlignedByteOffset;
		INPUT_CLASSIFICATION InputSlotClass;
		unsigned int InstanceDataStepRate;
		int Stride = 0;
	} 	VertexElementDESC;
	
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
	int GetNameHash();
	static bool IsShaderSupported_SM6(const ShaderComplieSettings& args);
	
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