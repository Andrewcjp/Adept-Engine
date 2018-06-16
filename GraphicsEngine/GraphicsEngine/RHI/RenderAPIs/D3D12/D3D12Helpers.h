#pragma once
#include <d3d12.h>
#include "../RHI/RHITypes.h"
class D3D12Helpers
{
public:

	static std::string StringFromFeatureLevel(D3D_FEATURE_LEVEL FeatureLevel);
	static std::string DXErrorCodeToString(HRESULT result);
	static DXGI_FORMAT ConvertFormat(eTEXTURE_FORMAT format);
	static D3D12_SRV_DIMENSION ConvertDimension(eTextureDimension Dim);
	static D3D12_DSV_DIMENSION ConvertDimensionDSV(eTextureDimension Dim);
	static D3D12_RTV_DIMENSION ConvertDimensionRTV(eTextureDimension Dim);
	static D3D12_RESOURCE_DIMENSION ConvertToResourceDimension(eTextureDimension Dim);
};

