
#include "RHITypes.h"
#include "Core/Utils/StringUtil.h"
#include "Shader.h"

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
	DepthTest = Depth;
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

void RHIPipeLineStateObject::Complie()
{

}

bool RHIPipeLineStateObject::Equals(RHIPipeLineStateObject * other)
{
	//todo:
	return false;
}

bool RHIPipeLineStateObject::IsReady() const
{
	return IsComplied;
}

const RHIPipeLineStateDesc & RHIPipeLineStateObject::GetDesc()
{
	return Desc;
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
	//todo: hash all members
	std::string Data = "";
	Data += ShaderInUse->GetName();
	Data += Blending;
	Data += Cull;
	Data += Mode;
	Data += std::to_string((int)RenderTargetDesc.RTVFormats[0]);
	UniqueHash = std::hash<std::string>{} (Data);
}

bool RHIPipeLineStateDesc::operator==(const RHIPipeLineStateDesc other) const
{
	//todo: way to get the complier to gen this?
	return ShaderInUse == other.ShaderInUse && Cull == other.Cull && RenderTargetDesc.RTVFormats[0] == other.RenderTargetDesc.RTVFormats[0] && Blending == other.Blending;
}

