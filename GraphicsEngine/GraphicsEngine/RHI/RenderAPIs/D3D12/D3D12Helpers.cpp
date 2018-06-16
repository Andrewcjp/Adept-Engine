#include "stdafx.h"
#include "D3D12Helpers.h"
#include "Core/Asserts.h"

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
	return (DXGI_FORMAT)format;
}

D3D12_SRV_DIMENSION D3D12Helpers::ConvertDimension(eTextureDimension Dim)
{
	return (D3D12_SRV_DIMENSION)Dim;
}
//todo:!
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
		NewDim = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	case eTextureDimension::DIMENSION_TEXTURE3D:
	case eTextureDimension::DIMENSION_TEXTURECUBEARRAY:
		NewDim = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	default:
		break;
	}
	return NewDim;
}