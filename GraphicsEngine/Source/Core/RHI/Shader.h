#pragma once
#include "Core/Transform.h"
#include "Rendering/Core/Camera.h"

#include "Rendering/Core/Light.h"
#include "ShaderProgramBase.h"
#include "Core/Assets/ShaderComplier.h"
//this is a basis for a shader 
//shaders will derive from this class so that the correct unifroms can be updated

//Static Object adds a Function ptr for the consructor to the shader complier
//Declare Permitation 
#define DECLARE_GLOBAL_SHADER(Type)\
Shader* ConstructCompiledInstance_##Type(const ShaderInit & Data)\
{\
	return new Type(Data.Context);\
}\
DECLARE_GLOBAL_SHADER_PERMIUTATION(Type,Type,void*,0)

#define DECLARE_GLOBAL_SHADER_ARGS(Type,Datatype)\
Shader* ConstructCompiledInstance_##Type(const ShaderInit & Data)\
{\
	return new Type(Data.Context,*((Datatype*)Data.Data));\
}\

#define DECLARE_GLOBAL_SHADER_PERMIUTATION(Name,Type,Datatype,DataValue)\
Datatype  _Type_##Name = DataValue; \
ShaderType Type_##Name = ShaderType(std::string(#Type) + std::to_string(DataValue), &ConstructCompiledInstance_##Type, ShaderInit(&_Type_##Name, sizeof(Datatype))); \

#define NAME_SHADER(Type) const std::string GetName() override{return #Type;}
class Shader
{
public:

	enum ShaderParamType { SRV, UAV, CBV, RootConstant };
	enum RHI_SHADER_VISIBILITY
	{
		SHADER_VISIBILITY_ALL = 0,
		SHADER_VISIBILITY_VERTEX = 1,
		SHADER_VISIBILITY_HULL = 2,
		SHADER_VISIBILITY_DOMAIN = 3,
		SHADER_VISIBILITY_GEOMETRY = 4,
		SHADER_VISIBILITY_PIXEL = 5
	};
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
	} 	VertexElementDESC;
	struct ShaderParameter
	{
		ShaderParameter()
		{}
		ShaderParameter(ShaderParamType it, int sigslot, int ShaderRegister, int ShaderRegSpace, RHI_SHADER_VISIBILITY Vis)
		{
			Type = it;
			SignitureSlot = sigslot;
			RegisterSlot = ShaderRegister;
			Visiblity = Vis;
			RegisterSpace = ShaderRegSpace;
		}
		ShaderParameter(ShaderParamType it, int sigslot, int ShaderRegister, int ShaderRegSpace = 0) :ShaderParameter(it, sigslot, ShaderRegister, ShaderRegSpace, RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL)
		{
			if (it == ShaderParamType::SRV)
			{
				Visiblity = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_PIXEL;
			}
		}
		ShaderParamType Type = ShaderParamType::CBV;
		RHI_SHADER_VISIBILITY Visiblity = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL;
		int SignitureSlot = 0;
		int RegisterSlot = 0;
		int NumDescriptors = 1;
		int RegisterSpace = 0;
	};
	//todo: migrate to New system
	typedef struct _MVPStruct
	{
		glm::mat4 M;
		glm::mat4 V;
		glm::mat4 P;
	}MVPStruct;
	Shader();
	Shader(DeviceContext* context);
	virtual ~Shader();
	virtual void SetShaderActive();
	RHI_API ShaderProgramBase* GetShaderProgram();
	virtual const std::string GetName();

	const int ShadowFarPlane = 500;
	virtual bool SupportsAPI(ERenderSystemType Type);
	virtual std::vector<ShaderParameter> GetShaderParameters();
	virtual std::vector<VertexElementDESC> GetVertexFormat();
	virtual bool IsComputeShader();
protected:
	ShaderProgramBase * m_Shader = nullptr;
	class DeviceContext* Device = nullptr;
};

const int ALBEDOMAP = 0;
const int SHADOWCUBEMAP = 10;
const int SHADOWCUBEMAP2 = 11;
const int SHADOWDIRMAP1 = 9;
const int NORMALMAP = 6;
const int DISPMAP = 5;