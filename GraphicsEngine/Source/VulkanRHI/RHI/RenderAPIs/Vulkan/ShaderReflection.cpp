#include "ShaderReflection.h"
#undef check
#include "Vulkan\glslang\Include\Types.h"

ShaderReflection::ShaderReflection()
{}


ShaderReflection::~ShaderReflection()
{}
ShaderParamType::Type ConvertType(const glslang::TBasicType& type)
{
	switch (type)
	{
		case glslang::EbtVoid:
		case glslang::EbtFloat:
		case glslang::EbtDouble:
		case glslang::EbtFloat16:
		case glslang::EbtInt8:
		case glslang::EbtUint8:
		case glslang::EbtInt16:
		case glslang::EbtUint16:
		case glslang::EbtInt:
		case glslang::EbtUint:
		case glslang::EbtInt64:
		case glslang::EbtUint64:
		case glslang::EbtBool:
		case glslang::EbtAtomicUint:
			return ShaderParamType::RootConstant;
		case glslang::EbtSampler:
			return ShaderParamType::Sampler;
			break;
		case glslang::EbtStruct:
			break;
		case glslang::EbtBlock:
			return ShaderParamType::CBV;
			break;
#ifdef NV_EXTENSIONS
		case glslang::EbtAccStructNV:
			return ShaderParamType::AccelerationStructure;
			break;
#endif
		case glslang::EbtReference:
			break;
		case glslang::EbtString:
			break;
		case glslang::EbtNumTypes:
			break;
	}
	ENUMCONVERTFAIL();
	return ShaderParamType::Limit;
}

void ShaderReflection::ReflectShader(glslang::TProgram * program, std::vector<ShaderParameter>& shaderbinds, bool & iscompute)
{
	program->buildReflection();
	for (int i = 0; i < program->getNumLiveUniformBlocks(); i++)
	{
		const glslang::TObjectReflection& Object = program->getUniformBlock(i);
		shaderbinds.push_back(CreateParam(Object));
	}
	//for (int i = 0; i < program->getNumLiveUniformVariables(); i++)
	//{
	//	const glslang::TObjectReflection& Object = program->getUniform(i);
	//	shaderbinds.push_back(CreateParam(Object));
	//}
	for (int i = 0; i < program->getNumBufferVariables(); i++)
	{
		const glslang::TObjectReflection& Object = program->getBufferVariable(i);
		shaderbinds.push_back(CreateParam(Object));
	}
	//program->dumpReflection();

}

ShaderParameter ShaderReflection::CreateParam(const glslang::TObjectReflection &Object)
{
	ShaderParameter Sp = {};
	Sp.Name = Object.name;
	Sp.RegisterSlot = Object.getBinding();
	Sp.NumDescriptors = Object.numMembers;
	Sp.Type = ConvertType(Object.getType()->getBasicType());
	return Sp;
}

#include "vulkan/spirv_cross/spirv_reflect.hpp"
ShaderParameter CreateParam_C(spirv_cross::CompilerReflection& comp, const spirv_cross::Resource& Res, ShaderParamType::Type sptype)
{
	ShaderParameter Sp = {};
	Sp.Name = Res.name;
	Sp.RegisterSpace = comp.get_decoration(Res.id, spv::DecorationDescriptorSet);
	Sp.RegisterSlot = comp.get_decoration(Res.id, spv::DecorationBinding);
	spirv_cross::Bitset buffer_flags = comp.get_buffer_block_flags(Res.id);
	if (sptype == ShaderParamType::SRV)
	{
		if (buffer_flags.get(spv::DecorationNonWritable))
		{
			Sp.Type = ShaderParamType::Buffer;
		}
		else
		{
			Sp.Type = ShaderParamType::UAV;
		}
	}
	else
	{
		Sp.Type = sptype;
	}
	return Sp;
}
void ShaderReflection::ReflectShader_SPVCross(std::vector<uint32_t>& OutSpirv, std::vector<ShaderParameter>& shaderbinds, bool & iscompute)
{
	spirv_cross::CompilerReflection comp(reinterpret_cast<const uint32_t*>(OutSpirv.data()), OutSpirv.size());

	spirv_cross::ShaderResources r = comp.get_shader_resources();
	for (int i = 0; i < r.storage_buffers.size(); i++)
	{
		const spirv_cross::Resource& Res = r.storage_buffers[i];
		shaderbinds.push_back(CreateParam_C(comp, Res, ShaderParamType::SRV));
	}
	for (int i = 0; i < r.push_constant_buffers.size(); i++)
	{
		const spirv_cross::Resource& Res = r.push_constant_buffers[i];
		shaderbinds.push_back(CreateParam_C(comp, Res, ShaderParamType::RootConstant));
	}
	for (int i = 0; i < r.uniform_buffers.size(); i++)
	{
		const spirv_cross::Resource& Res = r.uniform_buffers[i];
		shaderbinds.push_back(CreateParam_C(comp, Res, ShaderParamType::CBV));
	}
	for (int i = 0; i < r.sampled_images.size(); i++)
	{
		const spirv_cross::Resource& Res = r.sampled_images[i];
		shaderbinds.push_back(CreateParam_C(comp, Res, ShaderParamType::SRV));
	}
}

