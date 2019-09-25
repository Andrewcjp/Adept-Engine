#pragma once
#include "Vulkan/glslang/Public/ShaderLang.h"
#include "RHI/ShaderBase.h"
class ShaderReflection
{
public:
	ShaderReflection();
	~ShaderReflection();
	static void ReflectShader(glslang::TProgram* program, std::vector<ShaderParameter> & shaderbinds, bool & iscompute);

	static ShaderParameter CreateParam(const glslang::TObjectReflection &Object);

};

