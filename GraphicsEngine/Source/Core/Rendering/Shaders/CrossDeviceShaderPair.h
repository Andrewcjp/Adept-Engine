#pragma once
#include "RHI/ShaderProgramBase.h"

class Shader_Pair;
class CrossDeviceShaderPair
{
public:
	CrossDeviceShaderPair(const std::vector<std::string>& Names, const std::vector<EShaderType::Type>& StageList, const std::vector<ShaderProgramBase::Shader_Define>& Defines = std::vector<ShaderProgramBase::Shader_Define>());

	void InitOnDevice(int DeviceIndex);

	Shader_Pair* Get(DeviceContext* context = nullptr);
	Shader_Pair * Get(RHICommandList * list);
private:
	Shader_Pair* Pairs[MAX_GPU_DEVICE_COUNT];
	std::vector<std::string> Names;
	std::vector<EShaderType::Type> StageList;
	std::vector<ShaderProgramBase::Shader_Define> Defines;
};

