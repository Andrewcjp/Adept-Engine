#include "Shader_VRRResolve.h"

IMPLEMENT_GLOBAL_SHADER(Shader_VRRResolve);
Shader_VRRResolve::Shader_VRRResolve(DeviceContext * device) :Shader(device)
{
	m_Shader->AttachAndCompileShaderFromFile("VRX\\VRRResolve", EShaderType::SHADER_COMPUTE);
	DataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	DataBuffer->SetDebugName("Shader_VRSResolve DataBuffer");
	DataBuffer->CreateConstantBuffer(sizeof(VRSData), 1);
}

Shader_VRRResolve::~Shader_VRRResolve()
{}

void Shader_VRRResolve::BindBuffer(RHICommandList * list)
{
	DataInst.Resolution[0] = Screen::GetScaledRes().x;
	DataInst.Resolution[1] = Screen::GetScaledRes().y;
	DataInst.Debug = DebugRate.GetBoolValue();
	//	DataInst.ShowGrid = ShowGrid.GetBoolValue();
	DataBuffer->UpdateConstantBuffer(&DataInst);
	list->SetConstantBufferView(DataBuffer, 0, "Data");
}
