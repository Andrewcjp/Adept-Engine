#pragma once
#include "Vulkan/glslang/Public/ShaderLang.h"
#include "RHI/ShaderBase.h"
class ShaderReflection
{
public:
	ShaderReflection();
	~ShaderReflection();
	static ShaderParameter CreateParam(const glslang::TObjectReflection &Object);
	static void ReflectShader_SPVCross(std::vector<uint32_t>& OutSpirv,  std::vector<ShaderParameter>& shaderbinds, bool & iscompute);
};

