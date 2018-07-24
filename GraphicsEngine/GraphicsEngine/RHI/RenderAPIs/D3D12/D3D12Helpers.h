#pragma once
#include <d3d12.h>
#include "RHI/RHITypes.h"
#if defined(_DEBUG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{}
#endif
#define NAME_D3D12_OBJECT(x) SetName(x, L#x)

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
	static D3D12_COMMAND_LIST_TYPE ConvertListType(ECommandListType::Type type);
};

//Used to Copy Back resources for read
//Run outside the RHI
struct AssetPathRef;
class D3D12ReadBackCopyHelper
{
public:
	void WriteBackRenderTarget();
	D3D12ReadBackCopyHelper(class DeviceContext* context, class GPUResource* Target);
	void WriteToFile(AssetPathRef& Ref);
private:
	GPUResource * WriteBackResource = nullptr;
	GPUResource* Target = nullptr;
	DeviceContext* Device = nullptr;
	class D3D12CommandList* Cmdlist = nullptr;
	void* pData = nullptr;
};