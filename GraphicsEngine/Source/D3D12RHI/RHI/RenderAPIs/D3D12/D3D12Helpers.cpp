
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "D3D12CommandList.h"
#include "D3D12DeviceContext.h"
#include "Core/Utils/FileUtils.h"
#include "DescriptorHeap.h"
#include "Core/Assets/AssetManager.h"

void D3D12Helpers::NameRHIObject(DescriptorHeap * Object, IRHIResourse * resource, std::string OtherData)
{
#if NAME_RHI_PRIMS
	//Object->SetName(StringUtils::ConvertStringToWide(OtherData + resource->GetDebugName()).c_str());
#endif
}
void D3D12Helpers::NameRHIObject(GPUResource * Object, IRHIResourse * resource, std::string OtherData)
{
#if NAME_RHI_PRIMS
	//Object->SetName(StringUtils::ConvertStringToWide(OtherData + resource->GetDebugName()).c_str());
#endif
}
void D3D12Helpers::NameRHIObject(ID3D12Object * Object, IRHIResourse * resource, std::string OtherData)
{
#if NAME_RHI_PRIMS
	//NAME_D3D12_SetName_Convert(Object, OtherData + resource->GetDebugName());
#endif
}

std::string D3D12Helpers::StringFromFeatureLevel(D3D_FEATURE_LEVEL FeatureLevel)
{
	switch (FeatureLevel)
	{
	case D3D_FEATURE_LEVEL_12_1: return "12.1";
	case D3D_FEATURE_LEVEL_12_0: return "12.0";
	case D3D_FEATURE_LEVEL_11_1: return "11.1";
	case D3D_FEATURE_LEVEL_11_0: return "11.0";
	case D3D_FEATURE_LEVEL_10_1: return "10.1";
	case D3D_FEATURE_LEVEL_10_0: return "10.0";
	case D3D_FEATURE_LEVEL_9_3: return "9.3";
	case D3D_FEATURE_LEVEL_9_2: return "9.2";
	case D3D_FEATURE_LEVEL_9_1: return "9.1";
	default:
		return "Unknown";
	}
}

std::string D3D12Helpers::DXErrorCodeToString(HRESULT result)
{
	switch (result)
	{
	case DXGI_ERROR_INVALID_CALL:
		return std::string("DXGI_ERROR_INVALID_CALL");
	case DXGI_ERROR_NOT_FOUND:
		return std::string("DXGI_ERROR_NOT_FOUND");
	case DXGI_ERROR_MORE_DATA:
		return std::string("DXGI_ERROR_MORE_DATA");
	case DXGI_ERROR_UNSUPPORTED:
		return std::string("DXGI_ERROR_UNSUPPORTED");
	case DXGI_ERROR_DEVICE_REMOVED:
		return std::string("DXGI_ERROR_DEVICE_REMOVED");
	case DXGI_ERROR_DEVICE_HUNG:
		return std::string("DXGI_ERROR_DEVICE_HUNG");
	case DXGI_ERROR_DEVICE_RESET:
		return std::string("DXGI_ERROR_DEVICE_RESET");
	case DXGI_ERROR_WAS_STILL_DRAWING:
		return std::string("DXGI_ERROR_WAS_STILL_DRAWING");
	case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
		return std::string("DXGI_ERROR_FRAME_STATISTICS_DISJOINT");
	case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
		return std::string("DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE");
	case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
		return std::string("DXGI_ERROR_DRIVER_INTERNAL_ERROR");
	case DXGI_ERROR_NONEXCLUSIVE:
		return std::string("DXGI_ERROR_NONEXCLUSIVE");
	case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
		return std::string("DXGI_ERROR_NOT_CURRENTLY_AVAILABLE");
	case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
		return std::string("DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED");
	case DXGI_ERROR_REMOTE_OUTOFMEMORY:
		return std::string("DXGI_ERROR_REMOTE_OUTOFMEMORY");
	case DXGI_ERROR_ACCESS_LOST:
		return std::string("DXGI_ERROR_ACCESS_LOST");
	case DXGI_ERROR_WAIT_TIMEOUT:
		return std::string("DXGI_ERROR_WAIT_TIMEOUT");
	case DXGI_ERROR_SESSION_DISCONNECTED:
		return std::string("DXGI_ERROR_SESSION_DISCONNECTED");
	case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
		return std::string("DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE");
	case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
		return std::string("DXGI_ERROR_CANNOT_PROTECT_CONTENT");
	case DXGI_ERROR_ACCESS_DENIED:
		return std::string("DXGI_ERROR_ACCESS_DENIED");
	case DXGI_ERROR_NAME_ALREADY_EXISTS:
		return std::string("DXGI_ERROR_NAME_ALREADY_EXISTS");
	case DXGI_ERROR_MODE_CHANGE_IN_PROGRESS:
		return std::string("DXGI_ERROR_MODE_CHANGE_IN_PROGRESS");
	case DXGI_DDI_ERR_WASSTILLDRAWING:
		return std::string("DXGI_DDI_ERR_WASSTILLDRAWING");
	case DXGI_DDI_ERR_UNSUPPORTED:
		return std::string("DXGI_DDI_ERR_UNSUPPORTED");
	case DXGI_DDI_ERR_NONEXCLUSIVE:
		return std::string("DXGI_DDI_ERR_NONEXCLUSIVE");
	case D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
		return std::string("D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS");
	case D3D10_ERROR_FILE_NOT_FOUND:
		return std::string("D3D10_ERROR_FILE_NOT_FOUND");
	case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
		return std::string("D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS");
	case D3D11_ERROR_FILE_NOT_FOUND:
		return std::string("D3D11_ERROR_FILE_NOT_FOUND");
	case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
		return std::string("D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS");
	case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD:
		return std::string(
			"D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD");
	case E_UNEXPECTED:
		return std::string("E_UNEXPECTED");
	case E_NOTIMPL:
		return std::string("E_NOTIMPL");
	case E_OUTOFMEMORY:
		return std::string("E_OUTOFMEMORY");
	case E_INVALIDARG:
		return std::string("E_INVALIDARG");
	case E_NOINTERFACE:
		return std::string("E_NOINTERFACE");
	case E_POINTER:
		return std::string("E_POINTER");
	case E_HANDLE:
		return std::string("E_HANDLE");
	case E_ABORT:
		return std::string("E_ABORT");
	case E_FAIL:
		return std::string("E_FAIL");
	case E_ACCESSDENIED:
		return std::string("E_ACCESSDENIED");
	case S_FALSE:
		return std::string("S_FALSE");
	case S_OK:
		return std::string("S_OK");
	default:
		return std::to_string(result);
	}
}
//for now these are the same!
DXGI_FORMAT D3D12Helpers::ConvertFormat(eTEXTURE_FORMAT format)
{
	static_assert(DXGI_FORMAT::DXGI_FORMAT_YUY2 == eTEXTURE_FORMAT::FORMAT_YUY2, "");
	return (DXGI_FORMAT)format;
}

D3D12_SRV_DIMENSION D3D12Helpers::ConvertDimension(eTextureDimension Dim)
{
	//static_assert(D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE == eTextureDimension::DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE, "");
	return (D3D12_SRV_DIMENSION)Dim;
}
D3D12_UAV_DIMENSION D3D12Helpers::ConvertDimensionUAV(eTextureDimension Dim)
{
	//static_assert(D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE3D == eTextureDimension::DIMENSION_TEXTURE3D, "");
	return (D3D12_UAV_DIMENSION)Dim;
}
//#DX12 complete!
D3D12_DSV_DIMENSION D3D12Helpers::ConvertDimensionDSV(eTextureDimension Dim)
{
	D3D12_DSV_DIMENSION NewDim;
	switch (Dim)
	{

	case eTextureDimension::DIMENSION_TEXTURE2D:
		NewDim = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
		break;
	case eTextureDimension::DIMENSION_TEXTURE2DARRAY:
		NewDim = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		break;
	case eTextureDimension::DIMENSION_TEXTURECUBE:
		NewDim = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		break;
	default:
		NoImpl();
		break;
	}
	return NewDim;
}

D3D12_RTV_DIMENSION D3D12Helpers::ConvertDimensionRTV(eTextureDimension Dim)
{
	D3D12_RTV_DIMENSION NewDim;
	switch (Dim)
	{

	case eTextureDimension::DIMENSION_TEXTURE2D:
		NewDim = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
		break;
	case eTextureDimension::DIMENSION_TEXTURECUBE:
	case eTextureDimension::DIMENSION_TEXTURE2DARRAY:
		NewDim = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		break;
	case eTextureDimension::DIMENSION_TEXTURE3D:
		NewDim = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE3D;
		break;
	default:
		NoImpl();
		break;
	}
	return NewDim;
}


D3D12_RESOURCE_DIMENSION D3D12Helpers::ConvertToResourceDimension(eTextureDimension Dim)
{
	D3D12_RESOURCE_DIMENSION NewDim;
	switch (Dim)
	{
	case eTextureDimension::DIMENSION_TEXTURE2DARRAY:
	case eTextureDimension::DIMENSION_TEXTURE2D:
	case eTextureDimension::DIMENSION_TEXTURECUBE:
		NewDim = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	case eTextureDimension::DIMENSION_TEXTURE3D:
	case eTextureDimension::DIMENSION_TEXTURECUBEARRAY:
		NewDim = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	default:
		NoImpl();
		break;
	}
	return NewDim;
}

D3D12_COMMAND_LIST_TYPE D3D12Helpers::ConvertListType(ECommandListType::Type type)
{
	switch (type)
	{
	case ECommandListType::Graphics:
		return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	case ECommandListType::RayTracing:
	case ECommandListType::Compute:
		return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE;
	case ECommandListType::Copy:
		return D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY;
	default:
		NoImpl();
		break;
	}
	return D3D12_COMMAND_LIST_TYPE();
}

D3D12_RESOURCE_STATES D3D12Helpers::ConvertBufferResourceState(EBufferResourceState::Type intype)
{
	switch (intype)
	{
	case EBufferResourceState::Read:
		return (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	case EBufferResourceState::Non_PixelShader:
		return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE; 
	case EBufferResourceState::UnorderedAccess:
		return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case EBufferResourceState::IndirectArgs:
		return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case EBufferResourceState::CopySrc:
		return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE;
	case EBufferResourceState::CopyDST:
		return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;
	}
	return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
}

std::string D3D12Helpers::ResouceStateToString(D3D12_RESOURCE_STATES state)
{
	//#DX12: finish this!
	switch (state)
	{
	case D3D12_RESOURCE_STATE_COMMON:
		return "D3D12_RESOURCE_STATE_COMMON";
		break;
	case D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
		return "D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER";
		break;
	case D3D12_RESOURCE_STATE_INDEX_BUFFER:
		return "D3D12_RESOURCE_STATE_INDEX_BUFFER";
		break;
	case D3D12_RESOURCE_STATE_RENDER_TARGET:
		return "D3D12_RESOURCE_STATE_RENDER_TARGET";
		break;
	case D3D12_RESOURCE_STATE_UNORDERED_ACCESS:
		return "D3D12_RESOURCE_STATE_UNORDERED_ACCESS";
		break;
	case D3D12_RESOURCE_STATE_DEPTH_WRITE:
		return "D3D12_RESOURCE_STATE_DEPTH_WRITE";
		break;
	case D3D12_RESOURCE_STATE_DEPTH_READ:
		break;
	case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
		return "D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE";
		break;
	case D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE:
		return "D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE";
		break;
	case D3D12_RESOURCE_STATE_STREAM_OUT:
		break;
	case D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT:
		break;
	case D3D12_RESOURCE_STATE_COPY_DEST:
		return "D3D12_RESOURCE_STATE_COPY_DEST";
		break;
	case D3D12_RESOURCE_STATE_COPY_SOURCE:
		return "D3D12_RESOURCE_STATE_COPY_SOURCE";
		break;
	case D3D12_RESOURCE_STATE_RESOLVE_DEST:
		break;
	case D3D12_RESOURCE_STATE_RESOLVE_SOURCE:
		break;
#if WIN10_1809
	case D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE:
		break;
	case D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ:
		break;
	case D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE:
		return "D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE";
		break;
#endif
	case D3D12_RESOURCE_STATE_GENERIC_READ:
		break;
	case D3D12_RESOURCE_STATE_VIDEO_DECODE_READ:
		break;
	case D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE:
		break;
	case D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ:
		break;
	case D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE:
		break;

	default:
		break;
	}
	return std::string();
}

void D3D12ReadBackCopyHelper::WriteBackRenderTarget()
{
	if (Cmdlist == nullptr)
	{
		return;
	}
	Cmdlist->ResetList();
	D3D12_RESOURCE_STATES InitalState = Target->GetCurrentState();
	Target->SetResourceState(Cmdlist, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
	D3D12_RESOURCE_DESC Desc = Target->GetResource()->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;
	CD3DX12_BOX box(0, 0, (LONG)Desc.Width, Desc.Height);

	Device->GetDevice()->GetCopyableFootprints(&Desc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);
	CD3DX12_TEXTURE_COPY_LOCATION dest(WriteBackResource->GetResource(), renderTargetLayout);
	CD3DX12_TEXTURE_COPY_LOCATION src(Target->GetResource(), 0);
	Cmdlist->GetCommandList()->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);

	Target->SetResourceState(Cmdlist, InitalState);
	if (UseCopy)
	{
		Device->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
		Cmdlist->Execute(DeviceContextQueue::InterCopy);
		Device->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	}
	else
	{
		Cmdlist->Execute();
	}
}

D3D12ReadBackCopyHelper::D3D12ReadBackCopyHelper(DeviceContext* context, GPUResource* inTarget, bool Exclude /*= false*/)
{
	if (!Exclude)
	{
		Get()->Helpers.push_back(this);
	}
	if (context == nullptr || inTarget == nullptr)
	{
		return;
	}
	ensure(inTarget);
	Device = D3D12RHI::DXConv(context);
	if (UseCopy)
	{
		Cmdlist = D3D12RHI::DXConv(RHI::CreateCommandList(ECommandListType::Copy, context));
	}
	else
	{
		Cmdlist = D3D12RHI::DXConv(RHI::CreateCommandList(ECommandListType::Graphics, context));
	}

	Target = inTarget;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout[6];
	UINT64 pTotalBytes = 0;
	D3D12_RESOURCE_DESC Desc = Target->GetResource()->GetDesc();
	Device->GetDevice()->GetCopyableFootprints(&Desc, 0, Desc.DepthOrArraySize, 0, layout, nullptr, nullptr, &pTotalBytes);
	UINT64 textureSize = D3D12Helpers::Align(layout[0].Footprint.RowPitch * layout[0].Footprint.Height);

	// Create a buffer with the same layout as the render target texture.
	D3D12_RESOURCE_DESC crossAdapterDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_NONE);
	ID3D12Resource* Readback = nullptr;
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
		D3D12_HEAP_FLAG_NONE,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		ID_PASS(&Readback)));
	WriteBackResource = new GPUResource(Readback, D3D12_RESOURCE_STATE_COPY_DEST, Device);
	NAME_D3D12_OBJECT(Readback);
}

D3D12ReadBackCopyHelper::~D3D12ReadBackCopyHelper()
{
	//Only Destroyed at the end of the RHI Lifetime
	SafeRHIRelease(WriteBackResource);
	SafeRHIRelease(Cmdlist);
}

void D3D12ReadBackCopyHelper::WriteToFile(std::string Ref)
{
	if (Cmdlist == nullptr)
	{
		return;
	}
	Device->CPUWaitForAll();
	const bool DDS = false;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout[6];
	UINT64 pTotalBytes = 0;
	D3D12_RESOURCE_DESC Desc = Target->GetResource()->GetDesc();
	Device->GetDevice()->GetCopyableFootprints(&Desc, 0, Desc.DepthOrArraySize, 0, layout, nullptr, nullptr, &pTotalBytes);

	std::string path = Ref;

	FileUtils::CreateDirectoriesToFullPath(path);
#if NAME_RHI_PRIMS
	path.append(Target->GetDebugName());
#endif
	path.append("  ");
	path.append(GenericPlatformMisc::GetDateTimeString());

	for (int i = 0; i < 1/*Desc.DepthOrArraySize*/; i++)
	{
		std::string SubName = path + "_" + std::to_string(i);
		SaveData(pTotalBytes, i, &layout[i], SubName, DDS);
	}

}

void D3D12ReadBackCopyHelper::SaveData(UINT64 pTotalBytes, int subresouse, D3D12_PLACED_SUBRESOURCE_FOOTPRINT * layout, std::string &path, const bool DDS)
{
	const D3D12_RANGE emptyRange = {};
	D3D12_RANGE readRange = {};
	ThrowIfFailed(WriteBackResource->GetResource()->Map(subresouse, &readRange, reinterpret_cast<void**>(&pData + readRange.Begin)));//+begin?
	BYTE* RawData = (BYTE*)malloc(pTotalBytes);
	ZeroMemory(RawData, pTotalBytes);
	BYTE* source = static_cast<BYTE*>(pData);
	BYTE* WritePtr = RawData;
	int ChannelCount = 4;
	if (layout->Footprint.Format == DXGI_FORMAT_R8_UNORM)
	{
		ChannelCount = 1;
	}

	for (size_t i = 0; i < layout->Footprint.Height; i++)
	{
		memcpy(WritePtr, source, layout->Footprint.Width * ChannelCount); // for 4 bytes per pixel
		source += layout[0].Footprint.RowPitch;
		WritePtr += layout->Footprint.Width * ChannelCount;
	}
	WriteBackResource->GetResource()->Unmap(0, &emptyRange);

	//remove the Alpha Value from the texture for BMPs
	for (uint i = 0; i < layout->Footprint.Height*layout->Footprint.Width*ChannelCount; i += ChannelCount)
	{
		BYTE* Ptr = (RawData + i + 3);
		*Ptr = 255;
	}
	if (DDS)
	{
		path.append(".DDS");
	}
	else
	{
		path.append(".bmp");
	}
	//de align
#if 0
	SOIL_save_image(path.c_str(), DDS ? SOIL_SAVE_TYPE_DDS : SOIL_SAVE_TYPE_BMP, (int)layout->Footprint.Width, layout->Footprint.Height, ChannelCount, RawData);
#endif
}

D3D12ReadBackCopyHelper * D3D12ReadBackCopyHelper::Get()
{
	if (Instance == nullptr)
	{
		Instance = new D3D12ReadBackCopyHelper(nullptr, nullptr, true);
	}
	return Instance;
}

void D3D12ReadBackCopyHelper::SaveResource(int i)
{
	WriteBackRenderTarget();
	WriteToFile(AssetManager::DirectGetGeneratedDir());
}

void D3D12ReadBackCopyHelper::TriggerWriteBackAll()
{
	for (int i = 0; i < Helpers.size(); i++)
	{
		Helpers[i]->SaveResource(i);
	}
}

D3D12ReadBackCopyHelper* D3D12ReadBackCopyHelper::Instance = nullptr;
size_t D3D12Helpers::GetBytesPerPixel(DXGI_FORMAT fmt)
{
	return BitsPerPixel(fmt) / 8;
}

size_t D3D12Helpers::BitsPerPixel(_In_ DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		return 24;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		return 0;
	}
}



D3D12_RESOURCE_ALLOCATION_INFO D3D12Helpers::GetResourceSizeData(int width, int height, DXGI_FORMAT  format, D3D12_RESOURCE_DIMENSION dim, bool Depth, D3D12DeviceContext* c)
{
	if (format == DXGI_FORMAT_UNKNOWN)
	{
		return D3D12_RESOURCE_ALLOCATION_INFO();
	}
	if (c == nullptr)
	{
		c = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	}
	D3D12_RESOURCE_DESC d = {};
	d.Width = width;
	d.Height = height;
	d.DepthOrArraySize = 1;
	d.MipLevels = 1;
	d.Dimension = dim;
	d.Format = format;
	d.Alignment = 0;
	d.SampleDesc.Count = 1;

	d.Flags = (Depth ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	D3D12_RESOURCE_ALLOCATION_INFO i = c->GetDevice()->GetResourceAllocationInfo(0, 1, &d);
	return i;
}


std::string D3D12Helpers::SMToString(D3D_SHADER_MODEL SM)
{
	switch (SM)
	{
	case D3D_SHADER_MODEL_5_1:
		return "D3D_SHADER_MODEL_5_1";
	case D3D_SHADER_MODEL_6_0:
		return "D3D_SHADER_MODEL_6_0";
	case D3D_SHADER_MODEL_6_1:
		return "D3D_SHADER_MODEL_6_1";
#if WIN10_1809
	case D3D_SHADER_MODEL_6_2:
		return "D3D_SHADER_MODEL_6_2";
	case D3D_SHADER_MODEL_6_3:
		return "D3D_SHADER_MODEL_6_3";
	case D3D_SHADER_MODEL_6_4:
		return "D3D_SHADER_MODEL_6_4";
#endif
	}
	return "?";
}

ID3D12CommandAllocator* CommandAllocator::GetAllocator()
{
	return Allocators[Device->GetCpuFrameIndex()];
}

void CommandAllocator::Reset()
{
	//a device might run past the end of the CPU frame (or ahead)
	if (FrameReset == Device->GetDeviceFrame())
	{
		return;
	}
	ThrowIfFailed(Allocators[Device->GetCpuFrameIndex()]->Reset());
	FrameReset = Device->GetDeviceFrame();
}

CommandAllocator::CommandAllocator(ECommandListType::Type Type, D3D12DeviceContext* D)
{
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(D->GetDevice()->CreateCommandAllocator(D3D12Helpers::ConvertListType(Type), ID_PASS(&Allocators[i])));
	}
	AllocatorType = Type;
	Device = D;
}

CommandAllocator::~CommandAllocator()
{
	ensure(User == nullptr);
	MemoryUtils::DeleteReleaseableCArray(Allocators, RHI::CPUFrameCount);
}

void CommandAllocator::SetUser(D3D12CommandList * list)
{
	User = list;
}

bool CommandAllocator::IsInUse() const
{
	return User != nullptr;
}

ECommandListType::Type CommandAllocator::GetType() const
{
	return AllocatorType;
}


D3D12_INDIRECT_ARGUMENT_DESC D3D12Helpers::ConvertArg(INDIRECT_ARGUMENT_DESC desc)
{
	D3D12_INDIRECT_ARGUMENT_DESC dxarg;
	ZeroMemory(&dxarg, sizeof(dxarg));
	switch (desc.Type)
	{
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DRAW:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DISPATCH:
		dxarg.Type = D3D12RHIConfig::DispatchArgOverride;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
		dxarg.VertexBuffer.Slot = desc.VertexBuffer.Slot;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
		dxarg.VertexBuffer.Slot = desc.VertexBuffer.Slot;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_CONSTANT:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
		dxarg.Constant.DestOffsetIn32BitValues = desc.Constant.DestOffsetIn32BitValues;
		dxarg.Constant.Num32BitValuesToSet = desc.Constant.Num32BitValuesToSet;
		dxarg.Constant.RootParameterIndex = desc.Constant.RootParameterIndex;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		dxarg.ConstantBufferView.RootParameterIndex = desc.ConstantBufferView.RootParameterIndex;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		dxarg.ShaderResourceView.RootParameterIndex = desc.ShaderResourceView.RootParameterIndex;
		break;
	case INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW:
		dxarg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
		dxarg.UnorderedAccessView.RootParameterIndex = desc.UnorderedAccessView.RootParameterIndex;
		break;
	}
	return dxarg;
}

D3D12_QUERY_HEAP_TYPE D3D12Helpers::ConvertQuerryType(EGPUQueryType::Type type)
{
	switch (type)
	{
	case EGPUQueryType::BinaryOcclusion:
	case EGPUQueryType::Occlusion:
		return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	case EGPUQueryType::Timestamp:
		return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	case EGPUQueryType::Pipeline_Stats:
		return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
	}
	return D3D12_QUERY_HEAP_TYPE();
}

bool D3D12Helpers::IsValidForQueryHeap(D3D12_QUERY_HEAP_TYPE type, EGPUQueryType::Type QuerryType)
{
	switch (type)
	{
	case D3D12_QUERY_HEAP_TYPE_OCCLUSION:
		return QuerryType == EGPUQueryType::Occlusion || QuerryType == EGPUQueryType::BinaryOcclusion;
	case D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP:
	case D3D12_QUERY_HEAP_TYPE_TIMESTAMP:
		return QuerryType == EGPUQueryType::Timestamp;
	case D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS:
		return QuerryType == EGPUQueryType::Pipeline_Stats;
	case D3D12_QUERY_HEAP_TYPE_SO_STATISTICS:
		break;
	}
	return false;
}

D3D12_RESOURCE_STATES D3D12Helpers::ConvertRHIState(EResourceState::Type State)
{
	switch (State)
	{
	case EResourceState::Common:
		return D3D12_RESOURCE_STATE_COMMON;
	case EResourceState::ComputeUse:
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case EResourceState::PixelShader:
		return  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case EResourceState::RenderTarget:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case EResourceState::Non_PixelShader:
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case EResourceState::UAV:
		return D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case EResourceState::CopyDst:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case EResourceState::CopySrc:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
#if WIN10_1903
	case EResourceState::ShadingRateImage:
		return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
#endif
	}
	return D3D12_RESOURCE_STATE_COMMON;
}

D3D12_RESOURCE_STATES D3D12Helpers::ConvertRHIState_Safe(EResourceState::Type State, bool IsDepthResouce)
{
	D3D12_RESOURCE_STATES DXState = D3D12Helpers::ConvertRHIState(State);
	if (DXState == D3D12_RESOURCE_STATE_RENDER_TARGET && IsDepthResouce)
	{
		DXState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	return DXState;
}
