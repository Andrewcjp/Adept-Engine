
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
size_t IRHIResourse::GetSizeOnGPU()
{
	return 0;
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

RHIPipeLineStateObject::RHIPipeLineStateObject(const RHIPipeLineStateDesc & desc) :
	Desc(desc)
{}

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
	return;
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
#if 0
		if (ShaderInUse->GetName() != other.ShaderInUse->GetName())
		{
			return false;
		}
#else
		if (ShaderInUse->GetNameHash() != other.ShaderInUse->GetNameHash())
		{
			return false;
		}
#endif
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

size_t RHIUtils::BitsPerPixel(eTEXTURE_FORMAT fmt)
{
	switch (fmt)
	{
		case FORMAT_R32G32B32A32_TYPELESS:
		case FORMAT_R32G32B32A32_FLOAT:
		case FORMAT_R32G32B32A32_UINT:
		case FORMAT_R32G32B32A32_SINT:
			return 128;

		case FORMAT_R32G32B32_TYPELESS:
		case FORMAT_R32G32B32_FLOAT:
		case FORMAT_R32G32B32_UINT:
		case FORMAT_R32G32B32_SINT:
			return 96;

		case FORMAT_R16G16B16A16_TYPELESS:
		case FORMAT_R16G16B16A16_FLOAT:
		case FORMAT_R16G16B16A16_UNORM:
		case FORMAT_R16G16B16A16_UINT:
		case FORMAT_R16G16B16A16_SNORM:
		case FORMAT_R16G16B16A16_SINT:
		case FORMAT_R32G32_TYPELESS:
		case FORMAT_R32G32_FLOAT:
		case FORMAT_R32G32_UINT:
		case FORMAT_R32G32_SINT:
		case FORMAT_R32G8X24_TYPELESS:
		case FORMAT_D32_FLOAT_S8X24_UINT:
		case FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case FORMAT_X32_TYPELESS_G8X24_UINT:
		case FORMAT_Y416:
		case FORMAT_Y210:
		case FORMAT_Y216:
			return 64;

		case FORMAT_R10G10B10A2_TYPELESS:
		case FORMAT_R10G10B10A2_UNORM:
		case FORMAT_R10G10B10A2_UINT:
		case FORMAT_R11G11B10_FLOAT:
		case FORMAT_R8G8B8A8_TYPELESS:
		case FORMAT_R8G8B8A8_UNORM:
		case FORMAT_R8G8B8A8_UNORM_SRGB:
		case FORMAT_R8G8B8A8_UINT:
		case FORMAT_R8G8B8A8_SNORM:
		case FORMAT_R8G8B8A8_SINT:
		case FORMAT_R16G16_TYPELESS:
		case FORMAT_R16G16_FLOAT:
		case FORMAT_R16G16_UNORM:
		case FORMAT_R16G16_UINT:
		case FORMAT_R16G16_SNORM:
		case FORMAT_R16G16_SINT:
		case FORMAT_R32_TYPELESS:
		case FORMAT_D32_FLOAT:
		case FORMAT_R32_FLOAT:
		case FORMAT_R32_UINT:
		case FORMAT_R32_SINT:
		case FORMAT_R24G8_TYPELESS:
		case FORMAT_D24_UNORM_S8_UINT:
		case FORMAT_R24_UNORM_X8_TYPELESS:
		case FORMAT_X24_TYPELESS_G8_UINT:
		case FORMAT_R9G9B9E5_SHAREDEXP:
		case FORMAT_R8G8_B8G8_UNORM:
		case FORMAT_G8R8_G8B8_UNORM:
		case FORMAT_B8G8R8A8_UNORM:
		case FORMAT_B8G8R8X8_UNORM:
		case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case FORMAT_B8G8R8A8_TYPELESS:
		case FORMAT_B8G8R8A8_UNORM_SRGB:
		case FORMAT_B8G8R8X8_TYPELESS:
		case FORMAT_B8G8R8X8_UNORM_SRGB:
		case FORMAT_AYUV:
		case FORMAT_Y410:
		case FORMAT_YUY2:
			return 32;

		case FORMAT_P010:
		case FORMAT_P016:
			return 24;

		case FORMAT_R8G8_TYPELESS:
		case FORMAT_R8G8_UNORM:
		case FORMAT_R8G8_UINT:
		case FORMAT_R8G8_SNORM:
		case FORMAT_R8G8_SINT:
		case FORMAT_R16_TYPELESS:
		case FORMAT_R16_FLOAT:
		case FORMAT_D16_UNORM:
		case FORMAT_R16_UNORM:
		case FORMAT_R16_UINT:
		case FORMAT_R16_SNORM:
		case FORMAT_R16_SINT:
		case FORMAT_B5G6R5_UNORM:
		case FORMAT_B5G5R5A1_UNORM:
		case FORMAT_A8P8:
		case FORMAT_B4G4R4A4_UNORM:
			return 16;

		case FORMAT_NV12:
		case FORMAT_420_OPAQUE:
		case FORMAT_NV11:
			return 12;

		case FORMAT_R8_TYPELESS:
		case FORMAT_R8_UNORM:
		case FORMAT_R8_UINT:
		case FORMAT_R8_SNORM:
		case FORMAT_R8_SINT:
		case FORMAT_A8_UNORM:
		case FORMAT_AI44:
		case FORMAT_IA44:
		case FORMAT_P8:
			return 8;

		case FORMAT_R1_UNORM:
			return 1;

		case FORMAT_BC1_TYPELESS:
		case FORMAT_BC1_UNORM:
		case FORMAT_BC1_UNORM_SRGB:
		case FORMAT_BC4_TYPELESS:
		case FORMAT_BC4_UNORM:
		case FORMAT_BC4_SNORM:
			return 4;

		case FORMAT_BC2_TYPELESS:
		case FORMAT_BC2_UNORM:
		case FORMAT_BC2_UNORM_SRGB:
		case FORMAT_BC3_TYPELESS:
		case FORMAT_BC3_UNORM:
		case FORMAT_BC3_UNORM_SRGB:
		case FORMAT_BC5_TYPELESS:
		case FORMAT_BC5_UNORM:
		case FORMAT_BC5_SNORM:
		case FORMAT_BC6H_TYPELESS:
		case FORMAT_BC6H_UF16:
		case FORMAT_BC6H_SF16:
		case FORMAT_BC7_TYPELESS:
		case FORMAT_BC7_UNORM:
		case FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			return 0;
	}
}

size_t RHIUtils::GetPixelSize(eTEXTURE_FORMAT format)
{
	return BitsPerPixel(format) / 8;
}

RHISamplerDesc::RHISamplerDesc(ESamplerFilterMode::Type filter, ESamplerWrapMode::Type WrapMode, int Reg)
{
	FilterMode = filter;
	UAddressMode = WrapMode;
	VAddressMode = WrapMode;
	ShaderRegister = Reg;
}

std::vector<RHISamplerDesc> RHISamplerDesc::GetDefault()
{
	std::vector<RHISamplerDesc> Out;
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_MIN_MAG_MIP_LINEAR, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_WRAP, 0));
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_MIN_MAG_MIP_POINT, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_CLAMP, 1));
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_MIN_MAG_MIP_LINEAR, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_CLAMP, 2));
	return Out;
}

RHIRayDispatchDesc::RHIRayDispatchDesc(FrameBuffer * RB)
{
	Width = RB->GetWidth();
	Height = RB->GetHeight();
}

RHIRenderPassDesc::RHIRenderPassDesc(FrameBuffer * buffer, ERenderPassLoadOp::Type loadOp)
{
	LoadOp = loadOp;
	TargetBuffer = buffer;
}

void RHIRenderPassDesc::Build()
{
	if (TargetBuffer != nullptr)
	{
		RenderDesc = TargetBuffer->GetPiplineRenderDesc();
		InitalState = GPU_RESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET;
		//FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET;
	}
}

bool RHIRenderPassDesc::operator==(const RHIRenderPassDesc other) const
{
	return LoadOp == other.LoadOp && StoreOp == other.StoreOp && RenderDesc == other.RenderDesc && InitalState == other.InitalState && FinalState == other.FinalState;
}
