
#include "UpdateReflectionsNode.h"


UpdateReflectionsNode::UpdateReflectionsNode()
{}


UpdateReflectionsNode::~UpdateReflectionsNode()
{}

void UpdateReflectionsNode::OnExecute()
{
	/*CubemapCaptureList->ResetList();
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader());
	CubemapCaptureList->SetPipelineStateDesc(Desc);
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(CubemapCaptureList, true);
	}
	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		CubemapCaptureList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	SceneRender->UpdateRelflectionProbes(CubemapCaptureList);

	CubemapCaptureList->Execute();*/
}

void UpdateReflectionsNode::OnNodeSettingChange()
{
	
}

void UpdateReflectionsNode::OnSetupNode()
{
	
}
