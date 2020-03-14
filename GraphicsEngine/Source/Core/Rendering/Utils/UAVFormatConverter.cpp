#include "UAVFormatConverter.h"
#include "RHI/RHITexture.h"
#include "RHI/RHICommandList.h"
#include "RHI/ShaderBase.h"
#include "../Shaders/Shader_Pair.h"
UAVFormatConverter* UAVFormatConverter::instance = nullptr;

UAVFormatConverter::UAVFormatConverter()
{
	if (!RHI::GetDefaultDevice()->GetCaps().SupportTypedUAVLoads)
	{
		UnPackShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Utils\\UnPack" }, { EShaderType::SHADER_COMPUTE });
		PackShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Utils\\Pack" }, { EShaderType::SHADER_COMPUTE });
	}
}


UAVFormatConverter::~UAVFormatConverter()
{}

void UAVFormatConverter::UnPackToTmpResource(RHITexture ** ppRefTarget, RHICommandList * list, RHITexture * Target)
{
	if (list->GetDevice()->GetCaps().SupportTypedUAVLoads)
	{
		return;
	}
	RHITexture * RefTarget = *ppRefTarget;
	if (RefTarget == nullptr || RefTarget->GetDescription().Width != Target->GetDescription().Width || RefTarget->GetDescription().Height != Target->GetDescription().Width)
	{
		if (RefTarget)
		{
			EnqueueSafeRHIRelease(RefTarget);
		}
		RefTarget = RHI::GetRHIClass()->CreateTexture2();
		RHITextureDesc2 TmpDesc;
		TmpDesc.Depth = 2;
		TmpDesc.Width = Target->GetDescription().Width;
		TmpDesc.Height = Target->GetDescription().Height;
		ensure(Target->GetDescription().Format == eTEXTURE_FORMAT::FORMAT_R16G16B16A16_FLOAT);
		TmpDesc.Format = eTEXTURE_FORMAT::FORMAT_R32_UINT;
		TmpDesc.AllowUnorderedAccess = true;
		RefTarget->Create(TmpDesc, list->GetDevice());
	}
	RefTarget->SetState(list, EResourceState::Non_PixelShader);
	list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(Get()->UnPackShader));
	list->SetTexture2(Target, "InTex");
	list->SetUAV(RefTarget, "DstTexture");
	list->DispatchSized(Target->GetDescription().Width, Target->GetDescription().Height, 1);
	list->UAVBarrier(RefTarget);
	*ppRefTarget = RefTarget;
}

void UAVFormatConverter::PackBacktoResource(RHITexture * PackedTarget, RHICommandList * list, RHITexture * Target)
{
	if (list->GetDevice()->GetCaps().SupportTypedUAVLoads)
	{
		return;
	}
	list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(Get()->PackShader));
	list->SetUAV(Target, "FinalOut");
	list->SetUAV(PackedTarget, "PackedInput");
	list->DispatchSized(Target->GetDescription().Width, Target->GetDescription().Height, 1);
	list->UAVBarrier(Target);
}

UAVFormatConverter * UAVFormatConverter::Get()
{
	if (instance == nullptr)
	{
		instance = new UAVFormatConverter();
	}
	return instance;
}


