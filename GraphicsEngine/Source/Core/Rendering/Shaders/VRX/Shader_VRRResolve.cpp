#include "Shader_VRRResolve.h"
#include "..\..\Core\Screen.h"
#include "Core\Input\Input.h"
static ConsoleVariable DebugRateVRR("VRR.ShowRate", false, ECVarType::ConsoleAndLaunch);
static ConsoleVariable ShowGrid("VRR.ShowGrid", false, ECVarType::ConsoleAndLaunch);
static ConsoleVariable LerpBlend("VRR.Blend", 0.4f, ECVarType::ConsoleAndLaunch);

IMPLEMENT_GLOBAL_SHADER(Shader_VRRResolve);//todo: compile version with tilesize 8

#define VRR_FAST_RESOLVE 0
Shader_VRRResolve::Shader_VRRResolve(DeviceContext * device) : Shader(device)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(device->GetCaps().VRSTileSize)));
#if VRR_FAST_RESOLVE
	//m_Shader->AttachAndCompileShaderFromFile("VRX\\VRRResolveFast", EShaderType::SHADER_COMPUTE);
	m_Shader->AttachAndCompileShaderFromFile("VRX\\VRRResolveFast_LDS", EShaderType::SHADER_COMPUTE);
#else
	m_Shader->AttachAndCompileShaderFromFile("VRX\\VRRResolve", EShaderType::SHADER_COMPUTE);
#endif
	DataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	DataBuffer->SetDebugName("Shader_VRSResolve DataBuffer");
	DataBuffer->CreateConstantBuffer(sizeof(VRSData), 1);
	AddressCache = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	AddressCache->CreateConstantBuffer(sizeof(DataCahce), 1);
	DataInst = {};
	LerpBlend.SetValueF(1.0f);
}

Shader_VRRResolve::~Shader_VRRResolve()
{}

void Shader_VRRResolve::BindBuffer(RHICommandList * list)
{
	DataInst.Res = Screen::GetScaledRes();
	DataInst.Debug = DebugRateVRR.GetBoolValue();
	DataInst.LerpBlend = LerpBlend.GetFloatValue();
	DataInst.ShowGrid = ShowGrid.GetBoolValue();
	DataInst.EdgeHeat = RHI::GetRenderSettings()->GetDebugRenderMode() == ERenderDebugOutput::Scene_EdgeDetectCount;
	DataBuffer->UpdateConstantBuffer(&DataInst);
	list->SetConstantBufferView(DataBuffer, 0, "Data");
	glm::ivec2 Rate = glm::ivec2(4, 4);
	DataCahce.BufferLength = 0;
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			glm::ivec2 Pixel = glm::ivec2(x, y);
			glm::ivec2 DeltaToMain = Pixel % Rate;
			if (DeltaToMain == glm::ivec2(0, 0))
			{
				continue;
			}
			glm::ivec2 SourcePixel = Pixel.xy - DeltaToMain;
			//	AddressCache[index] = int4(-DeltaToMain, DTid.xy);
			DataCahce.DataSet[DataCahce.BufferLength] = glm::ivec4(-DeltaToMain, Pixel);
			DataCahce.BufferLength++;
		}
	}
	//DataCahce.BufferLength = 10;
	AddressCache->UpdateConstantBuffer(&DataCahce);
	list->SetConstantBufferView(AddressCache, 0, "AddressData");

}

bool Shader_VRRResolve::IsDebugActive() const
{
	return true;
	return DebugRateVRR.GetBoolValue() || ShowGrid.GetBoolValue() || RHI::GetRenderSettings()->GetDebugRenderMode() == ERenderDebugOutput::Scene_EdgeDetectCount;
}
