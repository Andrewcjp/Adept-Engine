#include "Shader_VRRResolve.h"
#include "..\..\Core\Screen.h"
static ConsoleVariable DebugRateVRR("VRR.ShowRate", 0, ECVarType::ConsoleAndLaunch);
IMPLEMENT_GLOBAL_SHADER(Shader_VRRResolve);
Shader_VRRResolve::Shader_VRRResolve(DeviceContext * device) : Shader(device)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(device->GetCaps().VRSTileSize)));
	m_Shader->AttachAndCompileShaderFromFile("VRX\\VRRResolve", EShaderType::SHADER_COMPUTE);
	DataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	DataBuffer->SetDebugName("Shader_VRSResolve DataBuffer");
	DataBuffer->CreateConstantBuffer(sizeof(VRSData), 1);
	//DebugRateVRR.SetValue(true);
}

Shader_VRRResolve::~Shader_VRRResolve()
{}

void Shader_VRRResolve::BindBuffer(RHICommandList * list)
{
	DataInst.Resolution[0] = Screen::GetScaledRes().x;
	DataInst.Resolution[1] = Screen::GetScaledRes().y;
	DataInst.Debug = DebugRateVRR.GetBoolValue();
	//	DataInst.ShowGrid = ShowGrid.GetBoolValue();
	DataBuffer->UpdateConstantBuffer(&DataInst);
	list->SetConstantBufferView(DataBuffer, 0, "Data");
}
