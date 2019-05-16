
#include "RHITypes.h"
#include "Core/Utils/StringUtil.h"
#include "Shader.h"
#include "Rendering/Core/FrameBuffer.h"

RHIFrameBufferDesc RHIFrameBufferDesc::CreateColour(int width, int height)
{
	RHIFrameBufferDesc newDesc = {};
	newDesc.Width = width;
	newDesc.Height = height;
	newDesc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
	return newDesc;
}

RHIFrameBufferDesc RHIFrameBufferDesc::CreateDepth(int width, int height)
{
	RHIFrameBufferDesc newDesc = {};
	newDesc.Width = width;
	newDesc.Height = height;
	newDesc.NeedsDepthStencil = true;
	newDesc.RenderTargetCount = 0;
	newDesc.Dimension = eTextureDimension::DIMENSION_TEXTURE2D;
	return newDesc;
}

RHIFrameBufferDesc RHIFrameBufferDesc::CreateCubeDepth(int width, int height)
{
	RHIFrameBufferDesc newDesc = CreateDepth(width, height);
	newDesc.Dimension = eTextureDimension::DIMENSION_TEXTURECUBE;
	newDesc.TextureDepth = 6;
	return newDesc;
}

RHIFrameBufferDesc RHIFrameBufferDesc::CreateCubeColourDepth(int width, int height)
{
	RHIFrameBufferDesc newDesc = CreateColourDepth(width, height);
	newDesc.Dimension = eTextureDimension::DIMENSION_TEXTURECUBE;
	newDesc.TextureDepth = 6;
	return newDesc;
}

RHIFrameBufferDesc RHIFrameBufferDesc::CreateColourDepth(int width, int height)
{
	RHIFrameBufferDesc newDesc = CreateColour(width, height);
	newDesc.NeedsDepthStencil = true;
	return newDesc;
}

RHIFrameBufferDesc RHIFrameBufferDesc::CreateGBuffer(int width, int height)
{
	RHIFrameBufferDesc newDesc = {};
	newDesc.Width = width;
	newDesc.Height = height;
	newDesc.RenderTargetCount = 3;
	newDesc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	newDesc.RTFormats[1] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_SNORM;
	newDesc.RTFormats[2] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_SNORM;
	//newDesc.RTFormats[3] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	newDesc.NeedsDepthStencil = true;
	return newDesc;
}

RHIPipeRenderTargetDesc RHIFrameBufferDesc::GetRTDesc()
{
	RHIPipeRenderTargetDesc d;
	d.NumRenderTargets = RenderTargetCount;
	for (int i = 0; i < MRT_MAX; i++)
	{
		d.RTVFormats[i] = RTFormats[i];
	}
	d.DSVFormat = DepthFormat;
	return d;
}

IRHIResourse::~IRHIResourse()
{
	DebugEnsure(IsReleased);
}

void IRHIResourse::Release()
{
	DebugEnsure(!IsReleased);
	IsReleased = true;
}
#if NAME_RHI_PRIMS
void IRHIResourse::SetDebugName(std::string Name)
{
	DebugName = StringUtils::CopyStringToCharArray(Name);
}

const char * IRHIResourse::GetDebugName()
{
	std::string Finaln = std::string(DebugName) + ObjectSufix;
	FinalName = StringUtils::CopyStringToCharArray(Finaln);
	return FinalName;
}
#else
void IRHIResourse::SetDebugName(std::string Name)
{}
#endif

void RHIPipeLineStateDesc::InitOLD(bool Depth, bool shouldcull, bool Blend)
{
	DepthStencilState.DepthEnable = Depth;
	Cull = shouldcull;
	Blending = Blend;
}

bool RHIPipeLineStateDesc::Validate()
{
	return true;
}

RHIPipeLineStateObject::RHIPipeLineStateObject(const RHIPipeLineStateDesc & desc)
{
	Desc = desc;
}

RHIPipeLineStateObject::~RHIPipeLineStateObject()
{}

size_t RHIPipeLineStateObject::GetDescHash()
{
	return Desc.GetHash();
}

std::string RHIPipeLineStateObject::GetDescString()
{
	return Desc.GetString();
}

void RHIPipeLineStateObject::Complie()
{

}

bool RHIPipeLineStateObject::IsReady() const
{
	return IsComplied;
}

const RHIPipeLineStateDesc & RHIPipeLineStateObject::GetDesc()
{
	return Desc;
}

DeviceContext* RHIPipeLineStateObject::GetDevice() const
{
	return Device;
}

void RHIPipeLineStateObject::Release()
{

}

size_t RHIPipeLineStateDesc::GetHash()
{
	if (UniqueHash == 0)
	{
		CalulateHash();
	}
	return UniqueHash;
}

void RHIPipeLineStateDesc::CalulateHash()
{
	//#RHI: hash all members
	StringPreHash = "";
	StringPreHash += ShaderInUse->GetName();
	StringPreHash += std::to_string(Blending);
	StringPreHash += std::to_string(Cull);
	StringPreHash += std::to_string(Mode);
	StringPreHash += std::to_string(DepthStencilState.DepthEnable);
	StringPreHash += std::to_string(DepthStencilState.DepthWrite);
	StringPreHash += std::to_string(DepthCompareFunction);
	for (int i = 0; i < MRT_MAX; i++)
	{
		StringPreHash += std::to_string((int)RenderTargetDesc.RTVFormats[i]);
	}
	UniqueHash = std::hash<std::string>{} (StringPreHash);
}

bool RHIPipeLineStateDesc::operator==(const RHIPipeLineStateDesc other) const
{
	//#RHI:way to get the complier to gen this?
	if (ShaderInUse != nullptr && other.ShaderInUse != nullptr)
	{
		if (ShaderInUse->GetName() != other.ShaderInUse->GetName())
		{
			return false;
		}
	}
	else if (ShaderInUse != other.ShaderInUse)
	{
		return false;
	}
	//#RHI: compare all props
	return Cull == other.Cull && DepthStencilState.DepthEnable == other.DepthStencilState.DepthEnable
		&& other.DepthCompareFunction == DepthCompareFunction
		&& RenderTargetDesc == other.RenderTargetDesc && Blending == other.Blending && other.DepthStencilState.DepthWrite == DepthStencilState.DepthWrite;
}

void RHIPipeLineStateDesc::Build()
{
	if (FrameBufferTarget != nullptr)
	{
		RenderTargetDesc = FrameBufferTarget->GetPiplineRenderDesc();
	}
	if (RenderTargetDesc.NumRenderTargets == 0 && RenderTargetDesc.DSVFormat == eTEXTURE_FORMAT::FORMAT_UNKNOWN)
	{
		//push the default
		RenderTargetDesc.NumRenderTargets = 1;
		RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
		RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	}
	CalulateHash();
}

std::string RHIPipeLineStateDesc::GetString()
{
	CalulateHash();
	return StringPreHash;
}

void SFRNode::AddBuffer(FrameBuffer * b)
{
	if (VectorUtils::Contains(NodesBuffers, b))
	{
		return;
	}
	NodesBuffers.push_back(b);
}

bool RHIPipeRenderTargetDesc::operator==(const RHIPipeRenderTargetDesc other) const
{
	for (int i = 0; i < MRT_MAX; i++)
	{
		if (other.RTVFormats[i] != RTVFormats[i])
		{
			return false;
		}
	}
	return other.NumRenderTargets == NumRenderTargets && other.DSVFormat == DSVFormat;
}
