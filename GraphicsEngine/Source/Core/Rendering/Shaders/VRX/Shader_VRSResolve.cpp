#include "Shader_VRSResolve.h"
#include "..\..\Core\Screen.h"
#include "..\..\Core\VRXEngine.h"

static ConsoleVariable DebugRate("VRS.ShowRate", 0, ECVarType::ConsoleAndLaunch);
static ConsoleVariable ShowGrid("VRS.ShowGrid", 1, ECVarType::ConsoleAndLaunch);
IMPLEMENT_GLOBAL_SHADER(Shader_VRSResolve);
Shader_VRSResolve::Shader_VRSResolve(DeviceContext * device) :Shader(device)
	, DataInst()
{
	//VRXEngine::SetupVRSShader(this, device);
	//m_Shader->AttachAndCompileShaderFromFile("VRX\\VRSResolve", EShaderType::SHADER_COMPUTE);
	//DataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	//DataBuffer->SetDebugName("Shader_VRSResolve DataBuffer");
	//DataBuffer->CreateConstantBuffer(sizeof(VRSData), 1);
}

Shader_VRSResolve::~Shader_VRSResolve()
{}

void Shader_VRSResolve::BindBuffer(RHICommandList * list)
{
	DataInst.Resolution[0] = Screen::GetScaledRes().x;
	DataInst.Resolution[1] = Screen::GetScaledRes().y;
	DataInst.Debug = DebugRate.GetBoolValue();
//	DataInst.ShowGrid = ShowGrid.GetBoolValue();
	DataBuffer->UpdateConstantBuffer(&DataInst);
	list->SetConstantBufferView(DataBuffer, 0, "Data");
}
