
#include "ReflectionProbe.h"
#include "RHI/RHI_inc.h"
#include "../Shaders/Shader_Depth.h"

ReflectionProbe::ReflectionProbe(glm::vec3 Pos/* = glm::vec3(0, 0, 0)*/)
{
	//ProbeMode = EReflectionProbeMode::ERealTime;
	int size = 1024;
	Dimentions.x = size;
	Dimentions.y = size;
	RHIFrameBufferDesc CubeDesc;
	CubeDesc = RHIFrameBufferDesc::CreateColourDepth(Dimentions.x, Dimentions.y);
	CubeDesc.Dimension = eTextureDimension::DIMENSION_TEXTURECUBE;
	CubeDesc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	CubeDesc.DepthFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	CubeDesc.TextureDepth = 6;
	CubeDesc.MipCount = 9;
	/*CubeDesc.DepthClearValue = 1.0f;*/
	CubeDesc.CubeMapAddressAsOne = false;
	CubeDesc.AllowUnorderedAccess = true;
	for (int d = 0; d < CubeDesc.TextureDepth; d++)
	{
		for (int i = 0; i < CubeDesc.MipCount; i++)
		{
			RHIViewDesc Desc;
			Desc.Mip = i;
			Desc.Dimention = DIMENSION_TEXTURE2DARRAY;
			Desc.ArraySlice = d;
			Desc.ViewType = EViewType::UAV;
			CubeDesc.RequestedViews.push_back(Desc);
		}
	}
	CapturedTexture = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), CubeDesc);
	CapturedTexture->SetDebugName("RelfectionProbe");

	ConvolutionRes = 64;
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateCubeColourDepth(ConvolutionRes, ConvolutionRes);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	ConvolutionBuffer = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), Desc);

	RelfectionProbeProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	RelfectionProbeProjections->CreateConstantBuffer(sizeof(MVBuffer), CUBE_SIDES, RHI::GetMGPUSettings()->InitSceneDataOnAllGPUs);
	Position = Pos;
	UpdateReflectionParams(Pos);
}

ReflectionProbe::~ReflectionProbe()
{
	EnqueueSafeRHIRelease(CapturedTexture);
	EnqueueSafeRHIRelease(RelfectionProbeProjections);
	EnqueueSafeRHIRelease(ConvolutionBuffer);
}

IntPoint ReflectionProbe::GetDimentions() const
{
	return Dimentions;
}

bool ReflectionProbe::NeedsCapture() const
{
	if (ProbeMode == EReflectionProbeMode::Baked)
	{
		return !IsCaptured;
	}
	return true;
}

void ReflectionProbe::SetCaptured()
{
	if (RHI::GetFrameCount() == 0)
	{
		return;
	}
	IsCaptured = true;
}

bool ReflectionProbe::GetNeedsDownSample() const
{
	return NeedsDownSample;
}

void ReflectionProbe::SetNeedsDownSample(bool val)
{
	NeedsDownSample = val;
}

void ReflectionProbe::BindViews(RHICommandList* List, int index, int slot)
{
	List->SetConstantBufferView(RelfectionProbeProjections, index, slot);
}

void ReflectionProbe::UpdateReflectionParams(glm::vec3 lightPos)
{
	glm::mat4x4 shadowProj = glm::perspectiveLH<float>(glm::radians(90.0f), 1.0f, zNear, ZFar);
	glm::mat4x4 transforms[6];
	transforms[0] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[1] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[2] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	transforms[3] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	transforms[4] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[5] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));
	for (int i = 0; i < CUBE_SIDES; i++)
	{
		CubeMapViews[i].P = shadowProj;
		CubeMapViews[i].V = transforms[i];
		CubeMapViews[i].CameraPos = lightPos;
	}
	for (int i = 0; i < CUBE_SIDES; i++)
	{
		RelfectionProbeProjections->UpdateConstantBuffer(&CubeMapViews[i], i);
	}
}

glm::vec3 ReflectionProbe::GetPosition() const
{
	return Position;
}

void ReflectionProbe::SetPosition(glm::vec3 val)
{
	Position = val;
	UpdateReflectionParams(Position);
}
