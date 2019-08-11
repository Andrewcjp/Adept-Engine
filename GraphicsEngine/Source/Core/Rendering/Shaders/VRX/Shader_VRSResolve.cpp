#include "Shader_VRSResolve.h"
#include "..\..\Core\Screen.h"

IMPLEMENT_GLOBAL_SHADER(Shader_VRSResolve);
Shader_VRSResolve::Shader_VRSResolve(DeviceContext * device) :Shader(device)
{
	m_Shader->AttachAndCompileShaderFromFile("VRX\\VRSResolve", EShaderType::SHADER_COMPUTE);
	DataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	DataBuffer->CreateConstantBuffer(sizeof(VRSData), 1);
}

Shader_VRSResolve::~Shader_VRSResolve()
{}
void Shader_VRSResolve::BindBuffer(RHICommandList * list)
{
	DataInst.Resolution[0] = Screen::GetScaledRes().x;
	DataInst.Resolution[1] = Screen::GetScaledRes().y;
	DataBuffer->UpdateConstantBuffer(&DataInst);
	list->SetConstantBufferView(DataBuffer, 0, "Data");
}
