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
};

