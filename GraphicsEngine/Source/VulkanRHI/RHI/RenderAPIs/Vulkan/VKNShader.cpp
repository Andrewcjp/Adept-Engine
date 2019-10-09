#include "VKNShader.h"


#include <ios>
#include <iostream>
#include <fstream>
#include "Core/Assets/AssetManager.h"
#include "VKNRHI.h"
#include "VKNDeviceContext.h"
#include "RHI/ShaderPreProcessor.h"
#include "Core/Platform/PlatformCore.h"
#include "ShaderReflection.h"




VKNShader::VKNShader()
{}


VKNShader::~VKNShader()
{}

EShaderError::Type VKNShader::AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint)
{
	if (type != EShaderType::SHADER_VERTEX && type != EShaderType::SHADER_FRAGMENT && type != EShaderType::SHADER_GEOMETRY && type != EShaderType::SHADER_COMPUTE)
	{
		return EShaderError::Type();
	}
	std::string Log = "Shader Compile Output: ";
	Log.append(filename);
	Log.append("\n");
	Log::LogMessage(Log);
	entyPoint = Entrypoint;
	std::vector<char> data = VKNShader::ComplieShader_Local(filename, type, true, filename);

	VkPipelineShaderStageCreateInfo I = {};
	I.module = createShaderModule(data);
	I.stage = ConvStage(type);
	I.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	I.pName = StringUtils::CopyStringToCharArray(entyPoint);
	Stages.push_back(I);
	return EShaderError::SHADER_ERROR_NONE;
}

std::vector<char> VKNShader::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
VkShaderModule VKNShader::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VKNRHI::GetVDefaultDevice()->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}


const TBuiltInResource DefaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,

	/* .limits = */ {
	/* .nonInductiveForLoops = */ 1,
	/* .whileLoops = */ 1,
	/* .doWhileLoops = */ 1,
	/* .generalUniformIndexing = */ 1,
	/* .generalAttributeMatrixVectorIndexing = */ 1,
	/* .generalVaryingIndexing = */ 1,
	/* .generalSamplerIndexing = */ 1,
	/* .generalVariableIndexing = */ 1,
	/* .generalConstantMatrixVectorIndexing = */ 1,
} };
class TIncluder : public glslang::TShader::Includer
{
public:
	virtual void releaseInclude(glslang::TShader::Includer::IncludeResult*) override
	{}
};
bool VKNShader::GenerateSpirv(const std::string Source, ComplieInfo & CompilerInfo, std::string & OutErrors, std::vector<char>& OutSpirv, std::string name)
{
	glslang::InitializeProcess();
	glslang::TProgram* Program = new glslang::TProgram();

	EShLanguage Stage = (CompilerInfo.stage);
	glslang::TShader* Shader = new glslang::TShader(Stage);
	const char* GlslSourceSkipHeader = Source.c_str();
	Shader->setStrings(&GlslSourceSkipHeader, 1);
	EShMessages Messages = (EShMessages)(EShMsgDefault | EShMsgDebugInfo | EShMsgSpvRules | EShMsgVulkanRules);
	if (CompilerInfo.HLSL)
	{
		Messages = (EShMessages)(Messages | EShMsgReadHlsl);
	}
	int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;
	if (CompilerInfo.HLSL)
	{
		Shader->setEnvInput(glslang::EShSourceHlsl, Stage, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	}
	else
	{
		Shader->setEnvInput(glslang::EShSourceGlsl, Stage, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	}
	Shader->setShiftBindingForSet(glslang::EResSampler, VKNShader::GetBindingOffset(ShaderParamType::Sampler), 0);
	Shader->setShiftBindingForSet(glslang::EResTexture, VKNShader::GetBindingOffset(ShaderParamType::SRV), 0);
	Shader->setShiftBindingForSet(glslang::EResUav, VKNShader::GetBindingOffset(ShaderParamType::UAV), 0);
	Shader->setShiftBindingForSet(glslang::EResUbo, VKNShader::GetBindingOffset(ShaderParamType::CBV), 0);
	Shader->setInvertY(true);
	Shader->setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
	Shader->setEnvTarget(glslang::EShTargetSpv, TargetVersion);
	Shader->setEntryPoint(entyPoint.c_str());

	TIncluder inc;
	const int DefaultVersion = 450;
	if (!Shader->parse(&DefaultTBuiltInResource, DefaultVersion, ENoProfile, false, false, Messages, inc))
	{
		Log::LogMessage(Program->getInfoLog());
		Log::LogMessage(Program->getInfoDebugLog());
		Log::LogMessage(Shader->getInfoLog());
		__debugbreak();
	}
	Program->addShader(Shader);
	if (!Program->link(Messages))
	{
		Log::LogMessage(Program->getInfoLog());
		Log::LogMessage(Program->getInfoDebugLog());
		__debugbreak();
	}
	Program->mapIO();
	if (!Program->getIntermediate(Stage))
	{
		Log::LogMessage(Program->getInfoDebugLog());
		__debugbreak();
	}
	
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = true;
	spvOptions.validate = true;
	spvOptions.generateDebugInfo = true;
	std::vector<uint32_t> WordSpirv;
	glslang::GlslangToSpv(*Program->getIntermediate((EShLanguage)Stage), WordSpirv, &logger, &spvOptions);

#if 1
	std::string root = AssetManager::GetShaderPath() + "\\VKan\\";
	if (CompilerInfo.HLSL)
	{
		root += "HLSL_";
	}
	StringUtils::RemoveChar(name, "\\");
	root += std::string(name);
	glslang::OutputSpvBin(WordSpirv, root.c_str());
#endif
	//convert word code to byte code - why is it the output
	for (int i = 0; i < WordSpirv.size(); i++)
	{
		char* BytePointer = (char*)&WordSpirv[i];
		OutSpirv.push_back(BytePointer[0]);
		OutSpirv.push_back(BytePointer[1]);
		OutSpirv.push_back(BytePointer[2]);
		OutSpirv.push_back(BytePointer[3]);
	}
	ShaderReflection::ReflectShader_SPVCross(WordSpirv, GeneratedParams, IsCompute);
	SafeDelete(Program);
	return true;
}
std::vector<char> VKNShader::ComplieShader(std::string name, EShaderType::Type T, bool HLSL /*= false*/)
{
	if (HLSL)
	{
		name.append(".hlsl");
	}
	std::string data = AssetManager::Get()->LoadFileWithInclude(name);
	std::string errors = "";
	std::vector<char> spirv = std::vector<char>();
	ComplieInfo t;
	t.stage = GetStage(T);
	t.HLSL = HLSL;
	GenerateSpirv(data, t, errors, spirv, "");
	return spirv;
}
std::vector<char> VKNShader::ComplieShader_Local(std::string name, EShaderType::Type T, bool HLSL, std::string ShaderDebugName)
{
	if (HLSL)
	{
		name.append(".hlsl");
	}
	std::string data = AssetManager::Get()->LoadFileWithInclude(name);
	std::string errors = "";
	std::vector<char> spirv = std::vector<char>();
	ComplieInfo t;
	t.stage = GetStage(T);
	t.HLSL = HLSL;
	ShaderPreProcessor::PreProcessDefines(Defines, data);
	GenerateSpirv(data, t, errors, spirv, ShaderDebugName);
	return spirv;
}

std::vector<VkPipelineShaderStageCreateInfo> VKNShader::GetShaderStages()
{
	return Stages;
}

EShLanguage VKNShader::GetStage(EShaderType::Type T)
{
	switch (T)
	{
		case EShaderType::SHADER_VERTEX:
			return EShLanguage::EShLangVertex;
			break;
		case EShaderType::SHADER_FRAGMENT:
			return EShLanguage::EShLangFragment;
			break;
		case EShaderType::SHADER_GEOMETRY:
			return EShLanguage::EShLangGeometry;
			break;
		case EShaderType::SHADER_COMPUTE:
			return EShLanguage::EShLangCompute;
			break;
		case EShaderType::SHADER_HULL:
			break;
		case EShaderType::SHADER_DOMAIN:
			break;
		case EShaderType::SHADER_RT_LIB:
			break;
	}
	ensure(false);
	return EShLangCount;
}

VkShaderStageFlagBits VKNShader::ConvStage(EShaderType::Type T)
{
	switch (T)
	{
		case EShaderType::SHADER_VERTEX:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case EShaderType::SHADER_FRAGMENT:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case EShaderType::SHADER_GEOMETRY:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		case EShaderType::SHADER_COMPUTE:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		case EShaderType::SHADER_HULL:
			break;
		case EShaderType::SHADER_DOMAIN:
			break;
		case EShaderType::SHADER_RT_LIB:
			break;
	}
	ensure(false);
	return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
}

int VKNShader::GetBindingOffset(ShaderParamType::Type Type)
{
	switch (Type)
	{
		case ShaderParamType::SRV:
			return 200;
			break;
		case ShaderParamType::UAV:
			return 0; //doesn't work
			break;
		case ShaderParamType::Buffer:
			return 0;//doen't work properly
		case ShaderParamType::CBV:
			return 100;
			break;
		case ShaderParamType::RootConstant:
			return 0;
			break;
		case ShaderParamType::Sampler:
			return 500;
			break;
		case ShaderParamType::Limit:
			break;
	}
	ensure(false);
	return 0;
}
