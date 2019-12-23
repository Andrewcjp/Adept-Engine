
#include "UpdateReflectionsNode.h"
#include "..\..\Core\Material.h"
#include "..\..\Core\SceneRenderer.h"
#include "..\..\Core\ReflectionEnviroment.h"
#include "..\..\Shaders\Shader_Main.h"
#include "../Core\Rendering\Core\SceneRenderer.h"
#include "Core\Assets\Scene.h"
#include "..\StorageNodeFormats.h"
#include "..\StoreNodes\ShadowAtlasStorageNode.h"
#include "RHI\DeviceContext.h"

UpdateReflectionsNode::UpdateReflectionsNode()
{
	SetNodeActive(false);
	OnNodeSettingChange();
}


UpdateReflectionsNode::~UpdateReflectionsNode()
{}

void UpdateReflectionsNode::OnExecute()
{
	//if (!SceneRenderer::Get()->GetReflectionEnviroment()->AnyProbesNeedUpdate())
	{
		return;
	}
	CubemapCaptureList->ResetList();
	SetBeginStates(CubemapCaptureList);
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader());
	CubemapCaptureList->SetPipelineStateDesc(Desc);
	//if (mShadowRenderer != nullptr)
	{
		//mShadowRenderer->BindShadowMapsToTextures(CubemapCaptureList, true);
	}
	GetShadowDataFromInput(0)->BindPointArray(CubemapCaptureList, MainShaderRSBinds::PointShadow);
	//	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox != nullptr)
	{
		CubemapCaptureList->SetTexture(SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	//CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	SceneRenderer::Get()->GetReflectionEnviroment()->UpdateRelflectionProbes(CubemapCaptureList);
	SetEndStates(CubemapCaptureList);
	CubemapCaptureList->Execute();
	FLAT_COMPUTE_START(RHI::GetDeviceContext(0));
	ComputeList->ResetList();
	SceneRenderer::Get()->GetReflectionEnviroment()->DownSampleAndBlurProbes(ComputeList);
	ComputeList->Execute();
	FLAT_COMPUTE_END(RHI::GetDeviceContext(0));

}

void UpdateReflectionsNode::OnNodeSettingChange()
{
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData);
}

void UpdateReflectionsNode::OnSetupNode()
{
	CubemapCaptureList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	ComputeList = RHI::CreateCommandList(ECommandListType::Compute, Context);
}

