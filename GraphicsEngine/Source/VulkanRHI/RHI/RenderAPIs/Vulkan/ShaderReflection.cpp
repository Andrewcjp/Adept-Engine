#include "ShaderReflection.h"
#undef check
#include "Vulkan\glslang\Include\Types.h"
#include "vulkan/spirv_cross/spirv_reflect.hpp"
#include "VKNShader.h"
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


ShaderParameter ShaderReflection::CreateParam(const glslang::TObjectReflection &Object)
{
	ShaderParameter Sp = {};
	Sp.Name = Object.name;
	Sp.RegisterSlot = Object.getBinding();
	Sp.NumDescriptors = Object.numMembers;
	Sp.Type = ConvertType(Object.getType()->getBasicType());
	return Sp;
}


ShaderParameter CreateParam_C(spirv_cross::CompilerReflection& comp, const spirv_cross::Resource& Res, ShaderParamType::Type sptype)
{
	ShaderParameter Sp = {};
	Sp.Name = Res.name;
	if (Res.name.size() == 0)
	{
		Sp.Name = comp.get_name(Res.base_type_id);
	}

	Sp.RegisterSpace = comp.get_decoration(Res.id, spv::DecorationDescriptorSet);
	Sp.RegisterSlot = comp.get_decoration(Res.id, spv::DecorationBinding);

	Sp.Type = sptype;
	if (sptype == ShaderParamType::SRV)
	{
		if (comp.get_type(Res.base_type_id).basetype == spirv_cross::SPIRType::Struct)
		{
			spirv_cross::Bitset buffer_flags = comp.get_buffer_block_flags(Res.id);

			if (buffer_flags.get(spv::DecorationNonWritable))
			{
				Sp.Type = ShaderParamType::Buffer;
			}
			else
			{
				Sp.Type = ShaderParamType::UAV;
			}
		}
	}
	Sp.RegisterSlot -= VKNShader::GetBindingOffset(Sp.Type);
	return Sp;
}
void ShaderReflection::ReflectShader_SPVCross(std::vector<uint32_t>& OutSpirv, std::vector<ShaderParameter>& shaderbinds, bool & iscompute)
{
	spirv_cross::CompilerReflection comp(reinterpret_cast<const uint32_t*>(OutSpirv.data()), OutSpirv.size());

	spirv_cross::ShaderResources r = comp.get_shader_resources();

	for (int i = 0; i < r.storage_buffers.size(); i++)
	{
		const spirv_cross::Resource& Res = r.storage_buffers[i];
		VectorUtils::AddUnique(shaderbinds, CreateParam_C(comp, Res, ShaderParamType::SRV));
	}

	for (int i = 0; i < r.push_constant_buffers.size(); i++)
	{
		const spirv_cross::Resource& Res = r.push_constant_buffers[i];
		VectorUtils::AddUnique(shaderbinds, CreateParam_C(comp, Res, ShaderParamType::RootConstant));
	}

	for (int i = 0; i < r.uniform_buffers.size(); i++)
	{
		const spirv_cross::Resource& Res = r.uniform_buffers[i];
		VectorUtils::AddUnique(shaderbinds, CreateParam_C(comp, Res, ShaderParamType::CBV));
	}

	for (int i = 0; i < r.sampled_images.size(); i++)
	{
		const spirv_cross::Resource& Res = r.sampled_images[i];
		VectorUtils::AddUnique(shaderbinds, CreateParam_C(comp, Res, ShaderParamType::SRV));
	}

	for (int i = 0; i < r.storage_images.size(); i++)
	{
		const spirv_cross::Resource& Res = r.storage_images[i];
		VectorUtils::AddUnique(shaderbinds, CreateParam_C(comp, Res, ShaderParamType::SRV));
	}

	for (int i = 0; i < r.separate_images.size(); i++)
	{
		const spirv_cross::Resource& Res = r.separate_images[i];
		VectorUtils::AddUnique(shaderbinds, CreateParam_C(comp, Res, ShaderParamType::SRV));
	}	
}

