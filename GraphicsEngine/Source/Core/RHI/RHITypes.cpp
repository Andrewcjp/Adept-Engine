#include "Core/Utils/StringUtil.h"
#include "Shader.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Core/Utils/TypeUtils.h"

RHIFrameBufferDesc RHIFrameBufferDesc::CreateColour(int width, int height)
{
	RHIFrameBufferDesc newDesc = {};
	newDesc.Width = width;
	newDesc.Height = height;
	newDesc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R16G16B16A16_FLOAT;
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
	newDesc.RTFormats[1] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
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

bool RHIFrameBufferDesc::HasStencil() const
{
	return DepthFormat == FORMAT_D24_UNORM_S8_UINT || DepthFormat == FORMAT_D32_FLOAT_S8X24_UINT;
}

IRHIResourse::~IRHIResourse()
{
	//	DebugEnsure(IsReleased);
}

void IRHIResourse::Release()
{
	DebugEnsure(!IsReleased);
	IsReleased = true;
}
size_t IRHIResourse::GetSizeOnGPU()
{
	return 0;
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
	UniqueHash = 0;
	HashUtils::hash_combine(UniqueHash, ShaderInUse->GetNameHash());
	HashUtils::hash_combine(UniqueHash, Cull);
	HashUtils::hash_combine(UniqueHash, DepthStencilState.DepthEnable);
	HashUtils::hash_combine(UniqueHash, DepthStencilState.DepthWrite);
	HashUtils::hash_combine(UniqueHash, DepthStencilState.DepthCompareFunction);
	for (int i = 0; i < MRT_MAX; i++)
	{
		HashUtils::hash_combine(UniqueHash, (int)RenderTargetDesc.RTVFormats[i]);
	}
	HashUtils::hash_combine(UniqueHash, (int)RenderTargetDesc.DSVFormat);
	HashUtils::hash_combine(UniqueHash, RenderPassDesc.GetHash());
	HashUtils::hash_combine(UniqueHash, RasterizerState.ConservativeRaster);
}

bool RHIPipeLineStateDesc::operator==(const RHIPipeLineStateDesc other) const
{
	//#RHI:way to get the complier to gen this?
	if (ShaderInUse != nullptr && other.ShaderInUse != nullptr)
	{
		if (ShaderInUse->GetNameHash() != other.ShaderInUse->GetNameHash())
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
		&& other.DepthStencilState.DepthCompareFunction == DepthStencilState.DepthCompareFunction
		&& RenderTargetDesc == other.RenderTargetDesc && other.DepthStencilState.DepthWrite == DepthStencilState.DepthWrite && RenderPassDesc == other.RenderPassDesc;
}

RHIPipeLineStateDesc RHIPipeLineStateDesc::CreateDefault(Shader* shader, FrameBuffer* FB /*= nullptr*/)
{
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = shader;
	if (FB != nullptr)
	{
		desc.RenderTargetDesc = FB->GetPiplineRenderDesc();
	}
	return desc;
}

void RHIPipeLineStateDesc::Build()
{
	RenderPassDesc.Build();
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

RHIPipeRenderTargetDesc RHIPipeRenderTargetDesc::GetDefault()
{
	RHIPipeRenderTargetDesc RenderTargetDesc = {};
	RenderTargetDesc.NumRenderTargets = 1;
	RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
	RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	return RenderTargetDesc;
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

size_t RHIUtils::GetComponentCount(eTEXTURE_FORMAT fmt)
{
	switch (fmt)
	{
	case FORMAT_R32G32B32A32_TYPELESS:
	case FORMAT_R32G32B32A32_FLOAT:
	case FORMAT_R32G32B32A32_UINT:
	case FORMAT_R32G32B32A32_SINT:
	case FORMAT_R16G16B16A16_TYPELESS:
	case FORMAT_R16G16B16A16_FLOAT:
	case FORMAT_R16G16B16A16_UNORM:
	case FORMAT_R16G16B16A16_UINT:
	case FORMAT_R16G16B16A16_SNORM:
	case FORMAT_R16G16B16A16_SINT:
	case FORMAT_R8G8B8A8_TYPELESS:
	case FORMAT_R8G8B8A8_UNORM:
	case FORMAT_R8G8B8A8_UNORM_SRGB:
	case FORMAT_R8G8B8A8_UINT:
	case FORMAT_R8G8B8A8_SNORM:
	case FORMAT_R8G8B8A8_SINT:
	case FORMAT_R10G10B10A2_TYPELESS:
	case FORMAT_R10G10B10A2_UNORM:
	case FORMAT_R10G10B10A2_UINT:
	case FORMAT_R8G8_B8G8_UNORM:
	case FORMAT_G8R8_G8B8_UNORM:
	case FORMAT_B8G8R8A8_UNORM:
	case FORMAT_B8G8R8X8_UNORM:
	case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case FORMAT_B8G8R8A8_TYPELESS:
	case FORMAT_B8G8R8A8_UNORM_SRGB:
	case FORMAT_B8G8R8X8_TYPELESS:
	case FORMAT_B8G8R8X8_UNORM_SRGB:
		return 4;

	case FORMAT_R32G32B32_TYPELESS:
	case FORMAT_R32G32B32_FLOAT:
	case FORMAT_R32G32B32_UINT:
	case FORMAT_R32G32B32_SINT:
	case FORMAT_R11G11B10_FLOAT:
		return 3;

	case FORMAT_R32G32_TYPELESS:
	case FORMAT_R32G32_FLOAT:
	case FORMAT_R32G32_UINT:
	case FORMAT_R32G32_SINT:
	case FORMAT_R32G8X24_TYPELESS:
	case FORMAT_R16G16_TYPELESS:
	case FORMAT_R16G16_FLOAT:
	case FORMAT_R16G16_UNORM:
	case FORMAT_R16G16_UINT:
	case FORMAT_R16G16_SNORM:
	case FORMAT_R16G16_SINT:
		return 2;

	case FORMAT_D32_FLOAT_S8X24_UINT:
	case FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case FORMAT_X32_TYPELESS_G8X24_UINT:
	case FORMAT_Y416:
	case FORMAT_Y210:
	case FORMAT_Y216:
		return 1;	
	
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
		return 1;
	case FORMAT_B5G6R5_UNORM:
	case FORMAT_B5G5R5A1_UNORM:
	case FORMAT_A8P8:
	case FORMAT_B4G4R4A4_UNORM:


	case FORMAT_NV12:
	case FORMAT_420_OPAQUE:
	case FORMAT_NV11:


	case FORMAT_R8_TYPELESS:
	case FORMAT_R8_UNORM:
	case FORMAT_R8_UINT:
	case FORMAT_R8_SNORM:
	case FORMAT_R8_SINT:
	case FORMAT_A8_UNORM:
	case FORMAT_AI44:
	case FORMAT_IA44:
	case FORMAT_P8:
	case FORMAT_R1_UNORM:
	case FORMAT_BC1_TYPELESS:
	case FORMAT_BC1_UNORM:
	case FORMAT_BC1_UNORM_SRGB:
	case FORMAT_BC4_TYPELESS:
	case FORMAT_BC4_UNORM:
	case FORMAT_BC4_SNORM:
		return 0;

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
	case FORMAT_AYUV:
	case FORMAT_Y410:
	case FORMAT_YUY2:
	case FORMAT_P010:
	case FORMAT_P016:
		return 0;

	default:
		return 0;
	}
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
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_ANISOTROPIC, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_WRAP, 0));
	Out[0].MaxAnisotropy = 16;
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_MIN_MAG_MIP_LINEAR, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_CLAMP, 1));
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_MIN_MAG_MIP_LINEAR, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_CLAMP, 2));
	Out.push_back(RHISamplerDesc(ESamplerFilterMode::FILTER_MIN_MAG_MIP_LINEAR, ESamplerWrapMode::TEXTURE_ADDRESS_MODE_CLAMP, 3));
	return Out;
}

RHIRayDispatchDesc::RHIRayDispatchDesc(FrameBuffer * RB)
{
	Width = RB->GetWidth();
	Height = RB->GetHeight();
	Target = RB;
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
	}
}

bool RHIRenderPassDesc::operator==(const RHIRenderPassDesc other) const
{
	return LoadOp == other.LoadOp && StoreOp == other.StoreOp && RenderDesc == other.RenderDesc && InitalState == other.InitalState && FinalState == other.FinalState && other.TargetBuffer == TargetBuffer;
}

size_t RHIRenderPassDesc::GetHash()
{
	size_t hash = 0;
	HashUtils::hash_combine(hash, LoadOp);
	HashUtils::hash_combine(hash, StoreOp);
	HashUtils::hash_combine(hash, InitalState);
	HashUtils::hash_combine(hash, FinalState);
	HashUtils::hash_combine(hash, TargetBuffer);
	return hash;
}

bool RHIViewDesc::operator==(const RHIViewDesc other) const
{
	return ArraySlice == other.ArraySlice &&
		Mip == other.Mip &&
		MipLevels == other.MipLevels &&
		ResourceIndex == other.ResourceIndex &&
		ViewType == other.ViewType &&
		Dimension == other.Dimension;
}

bool FrameBufferVariableRateSettings::UsesVRR() const
{
	return BufferMode == VRR && RHI::GetRenderSettings()->GetVRXSettings().UseVRR();
}

AccelerationStructureDesc::AccelerationStructureDesc()
{
	BuildFlags = AS_BUILD_FLAGS::Fast_Trace;
}

RHICommandSigniture::RHICommandSigniture(DeviceContext * context, RHICommandSignitureDescription desc)
{
	RHIdesc = desc;
	Context = context;
}

const RHICommandSignitureDescription & RHICommandSigniture::GetDesc() const
{
	return RHIdesc;
}

std::string EResourceState::ToString(EResourceState::Type state)
{
	switch (state)
	{
	case EResourceState::RenderTarget:
		return "RenderTarget";
	case EResourceState::PixelShader:
		return "PixelShader";;
	case EResourceState::ComputeUse:
		return "ComputeUse";
	case EResourceState::UAV:
		return "UAV";
	case EResourceState::CopySrc:
		return "CopySrc";
	case EResourceState::CopyDst:
		return "CopyDst";
	case EResourceState::Non_PixelShader:
		return "Non_PixelShader";
	case EResourceState::Undefined:
	case EResourceState::Limit:
		break;
	}

	return  "Undefined";
}

bool EResourceState::IsStateValidForList(ECommandListType::Type listtype, EResourceState::Type state)
{
	const bool ValidCopy = state == CopySrc || state == CopyDst || state == Common;
	if (listtype == ECommandListType::Graphics)
	{
		return true;
	}
	else if (listtype == ECommandListType::Compute || listtype == ECommandListType::RayTracing)
	{
		return ValidCopy || state == ComputeUse || state == Non_PixelShader || state == UAV ;
	}
	else if (listtype == ECommandListType::Copy)
	{
		return ValidCopy;
	}
	return false;
}

RHIBlendState RHIBlendState::CreateText()
{
	RHIBlendState BlendState = {};
	BlendState.RenderTargetDescs[0].BlendEnable = true;
	BlendState.RenderTargetDescs[0].BlendOpAlpha = RHIBlendOp::BLEND_OP_ADD;

	BlendState.RenderTargetDescs[0].SrcBlend = RHIBlendMode::BLEND_SRC_ALPHA;
	BlendState.RenderTargetDescs[0].DestBlend = RHIBlendMode::BLEND_INV_SRC_ALPHA;
	return BlendState;
}

RHIBlendState RHIBlendState::CreateBlendDefault()
{
	RHIBlendState BlendState = {};
	BlendState.RenderTargetDescs[0].BlendEnable = true;
	BlendState.RenderTargetDescs[0].BlendOp = RHIBlendOp::BLEND_OP_ADD;
	BlendState.RenderTargetDescs[0].SrcBlend = RHIBlendMode::BLEND_SRC_ALPHA;
	BlendState.RenderTargetDescs[0].DestBlend = RHIBlendMode::BLEND_INV_SRC_ALPHA;
	BlendState.RenderTargetDescs[0].SrcBlendAlpha = RHIBlendMode::BLEND_ONE;
	BlendState.RenderTargetDescs[0].DestBlendAlpha = RHIBlendMode::BLEND_INV_SRC_ALPHA;
	return BlendState;
}

DeviceContextQueue::Type DeviceContextQueue::GetFromCommandListType(ECommandListType::Type listType)
{
	switch (listType)
	{
	case ECommandListType::Graphics:
		return Graphics;
	case ECommandListType::Compute:
	case ECommandListType::RayTracing:
		return Compute;
	case ECommandListType::Copy:
		return Copy;
	case ECommandListType::VideoEncode:
	case ECommandListType::VideoDecode:
	case ECommandListType::Limit:
	default:
		break;
	}
	return DeviceContextQueue::LIMIT;
}
