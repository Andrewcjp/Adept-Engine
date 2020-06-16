#ifndef __SHADERBASE_H__
#define __SHADERBASE_H__

namespace EShaderType
{
	enum Type
	{
		SHADER_VERTEX,
		SHADER_FRAGMENT,
		SHADER_GEOMETRY,
		SHADER_COMPUTE,
		SHADER_HULL,
		SHADER_DOMAIN,
		SHADER_RT_LIB,
		SHADER_MESH,
		SHADER_AMPLIFICATION,
		SHADER_UNDEFINED
	};
}
namespace EShaderError
{
	enum Type
	{
		SHADER_ERROR_NONE = 0,
		SHADER_ERROR_NOFILE,
		SHADER_ERROR_CREATE,
		SHADER_ERROR_COMPILE,
		SHADER_ERROR_UNSUPPORTED,
		SHADER_ERROR_Compiler_INTERNAL,
		SHADER_ERROR_MAXCOUNT
	};
}
namespace ShaderParamType
{
	enum Type
	{
		SRV,
		Buffer,
		RootSRV,
		UAV,
		CBV,
		RootConstant,
		Sampler,
		AccelerationStructure,
		Limit
	};
}
enum RHI_SHADER_VISIBILITY
{
	SHADER_VISIBILITY_ALL = 0,
	SHADER_VISIBILITY_VERTEX = 1,
	SHADER_VISIBILITY_HULL = 2,
	SHADER_VISIBILITY_DOMAIN = 3,
	SHADER_VISIBILITY_GEOMETRY = 4,
	SHADER_VISIBILITY_PIXEL = 5
};
struct ShaderParameter
{
	ShaderParameter()
	{}
	ShaderParameter(ShaderParamType::Type it, int sigslot, int ShaderRegister, int ShaderRegSpace, RHI_SHADER_VISIBILITY Vis)
	{
		Type = it;
		SignitureSlot = sigslot;
		RegisterSlot = ShaderRegister;
		Visiblity = Vis;
		RegisterSpace = ShaderRegSpace;
	}
	ShaderParameter(ShaderParamType::Type it, int sigslot, int ShaderRegister, int ShaderRegSpace = 0) :ShaderParameter(it, sigslot, ShaderRegister, ShaderRegSpace, RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL)
	{
		if (it == ShaderParamType::SRV)
		{
			Visiblity = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_PIXEL;
		}
	}
	ShaderParamType::Type Type = ShaderParamType::Limit;
	int Visiblity = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL;
	int SignitureSlot = 0;
	int RegisterSlot = 0;
	int NumDescriptors = 1;
	int RegisterSpace = 0;
	std::string Name;
	bool operator==(const ShaderParameter & A)const
	{
		return A.Type == Type && A.RegisterSlot == RegisterSlot && A.RegisterSpace == RegisterSpace && A.NumDescriptors == NumDescriptors && A.Name == Name;
	}
	int NumVariablesContained = 1;
};

#endif
