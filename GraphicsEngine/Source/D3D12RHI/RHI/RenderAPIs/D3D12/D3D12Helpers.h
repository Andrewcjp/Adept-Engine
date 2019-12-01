#pragma once
#include "RHI/RHITypes.h"
#include "Core/Utils/StringUtil.h"

class D3D12DeviceContext;

#if NAME_RHI_PRIMS
inline void NAME_D3D12_SetName_Convert(ID3D12Object* pObject, std::string name)
{
	pObject->SetName(StringUtils::ConvertStringToWide(name).c_str());
}
inline void NAME_D3D12_SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
#define NAME_D3D12_OBJECT(x) NAME_D3D12_SetName(x, L#x)
#define NAME_RHI_OBJ(x) D3D12Helpers::NameRHIObject(x,this,#x);
#define REF_CHECK(x) if(x != nullptr){x->AddRef();int t = x->Release();ensure(t == 1)};
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{}
#define NAME_D3D12_OBJECT(x)
#define NAME_RHI_OBJ(x)
#define REF_CHECK(x)
#endif
#define SafeRelease(Target) if(Target != nullptr){Target->Release(); Target= nullptr;}
class D3D12Helpers
{
public:
	static void NameRHIObject(class DescriptorHeap * Object, IRHIResourse * resource, std::string OtherData = std::string());
	static void NameRHIObject(class GPUResource* Object, IRHIResourse* resource, std::string OtherData = std::string());
	static void NameRHIObject(ID3D12Object* Object, IRHIResourse* resource, std::string OtherData = std::string());
	static std::string StringFromFeatureLevel(D3D_FEATURE_LEVEL FeatureLevel);
	static std::string DXErrorCodeToString(HRESULT result);
	static DXGI_FORMAT ConvertFormat(eTEXTURE_FORMAT format);
	static D3D12_SRV_DIMENSION ConvertDimension(eTextureDimension Dim);
	static D3D12_DSV_DIMENSION ConvertDimensionDSV(eTextureDimension Dim);
	static D3D12_RTV_DIMENSION ConvertDimensionRTV(eTextureDimension Dim);
	static D3D12_RESOURCE_DIMENSION ConvertToResourceDimension(eTextureDimension Dim);
	static D3D12_COMMAND_LIST_TYPE ConvertListType(ECommandListType::Type type);
	static D3D12_RESOURCE_STATES ConvertBufferResourceState(EBufferResourceState::Type intype);
	static UINT64 Align(UINT64 size, UINT64 alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
	static std::string ResouceStateToString(D3D12_RESOURCE_STATES state);
	static size_t GetBytesPerPixel(DXGI_FORMAT fmt);
	static size_t BitsPerPixel(DXGI_FORMAT fmt);
	static D3D12_RESOURCE_ALLOCATION_INFO GetResourceSizeData(int width, int height, DXGI_FORMAT format, D3D12_RESOURCE_DIMENSION dim, bool depth = false, D3D12DeviceContext * c = nullptr);
	static std::string SMToString(D3D_SHADER_MODEL SM);
	static D3D12_INDIRECT_ARGUMENT_DESC ConvertArg(INDIRECT_ARGUMENT_DESC desc);
	static D3D12_QUERY_HEAP_TYPE ConvertQuerryType(EGPUQueryType::Type type);
	static bool IsValidForQueryHeap(D3D12_QUERY_HEAP_TYPE type, EGPUQueryType::Type QuerryType);
};

//Used to Copy Back resources for read
//Run outside the RHI
struct AssetPathRef;
class DeviceContext;
class GPUResource;
class D3D12ReadBackCopyHelper
{
public:
	void WriteBackRenderTarget();
	D3D12ReadBackCopyHelper(DeviceContext* context, GPUResource* Target, bool Exclude = false);
	~D3D12ReadBackCopyHelper();
	void WriteToFile(std::string Ref);

	void SaveData(UINT64 pTotalBytes, int subresouse, D3D12_PLACED_SUBRESOURCE_FOOTPRINT * layout, std::string & path, const bool DDS);

	static D3D12ReadBackCopyHelper* Get();
	void SaveResource(int i);
	void TriggerWriteBackAll();
private:
	bool UseCopy = false;
	static D3D12ReadBackCopyHelper* Instance;
	GPUResource * WriteBackResource = nullptr;
	GPUResource* Target = nullptr;
	class D3D12DeviceContext* Device = nullptr;
	class D3D12CommandList* Cmdlist = nullptr;
	void* pData = nullptr;
	std::vector<D3D12ReadBackCopyHelper*> Helpers;
};

class CommandAllocator
{
public:
	ID3D12CommandAllocator* GetAllocator();
	void Reset();
	CommandAllocator(ECommandListType::Type Type, D3D12DeviceContext * D);
	~CommandAllocator();
private:
	uint FrameReset = 0;
	DeviceContext* Device = nullptr;
	ID3D12CommandAllocator* Allocators[RHI::CPUFrameCount] = { nullptr,nullptr };
};