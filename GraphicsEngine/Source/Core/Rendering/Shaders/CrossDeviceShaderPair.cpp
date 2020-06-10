#include "CrossDeviceShaderPair.h"
#include "RHI/DeviceContext.h"
#include "Shader_Pair.h"

CrossDeviceShaderPair::CrossDeviceShaderPair(const std::vector<std::string>& Names, const std::vector<EShaderType::Type>& StageList, const std::vector<ShaderProgramBase::Shader_Define>& Defines)
{
	this->Names = Names;
	this->StageList = StageList;
	this->Defines = Defines;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		InitOnDevice(i);
	}
}

void CrossDeviceShaderPair::InitOnDevice(int DeviceIndex)
{
	Pairs[DeviceIndex] = new Shader_Pair(RHI::GetDeviceContext(DeviceIndex), Names, StageList, Defines);
}

Shader_Pair * CrossDeviceShaderPair::Get(DeviceContext * context)
{
	if (context == nullptr)
	{
		return Pairs[0];
	}
	return Pairs[context->GetDeviceIndex()];
}

Shader_Pair * CrossDeviceShaderPair::Get(RHICommandList* list)
{
	if (list == nullptr)
	{
		return Pairs[0];
	}
	return Pairs[list->GetDeviceIndex()];
}
