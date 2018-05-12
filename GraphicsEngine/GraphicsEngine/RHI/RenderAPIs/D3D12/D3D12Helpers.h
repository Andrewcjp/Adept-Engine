#pragma once
#include <d3d12.h>
class D3D12Helpers
{
public:

	static std::string StringFromFeatureLevel(D3D_FEATURE_LEVEL FeatureLevel);
	static std::string DXErrorCodeToString(HRESULT result);
};

