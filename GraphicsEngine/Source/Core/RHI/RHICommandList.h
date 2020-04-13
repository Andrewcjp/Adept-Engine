#pragma once
#include "RHITypes.h"
#include "RHITemplates.h"
#include "RHITexture.h"


class DeviceContext;
class HighLevelAccelerationStructure;
class RHIStateObject;
struct RHIBufferDesc
{
	int ElementCount = 0;
	int Stride = 0;
	int CounterSize = 0;
	EBufferAccessType::Type Accesstype;
	bool AllowUnorderedAccess = false;
	bool CreateSRV = false;
	bool CreateUAV = false;
	bool UseForExecuteIndirect = false;
	bool UseForReadBack = false;
	EResourceState::Type StartState = EResourceState::Undefined;
};

class  RHIBuffer : public IRHIResourse, public IRHISharedDeviceObject<RHIBuffer>
{
public:

	ERHIBufferType::Type CurrentBufferType;
	RHI_API RHIBuffer(ERHIBufferType::Type type);
	RHI_API RHI_VIRTUAL void CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype = EBufferAccessType::Static) = 0;
	RHI_API RHI_VIRTUAL void CreateBuffer(RHIBufferDesc Desc) = 0;
	RHI_API RHI_VIRTUAL void CreateIndexBuffer(int Stride, int ByteSize) = 0;
	RHI_API RHI_VIRTUAL void CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices = false) = 0;
	RHI_API RHI_VIRTUAL void UpdateConstantBuffer(void * data, int offset = 0) = 0;
	RHI_API RHI_VIRTUAL void UpdateVertexBuffer(void* data, size_t length, int VertexCount = -1) = 0;
	RHI_API RHI_VIRTUAL void UpdateIndexBuffer(void* data, size_t length) = 0;
	RHI_API RHI_VIRTUAL void BindBufferReadOnly(class RHICommandList* list, int RSSlot) = 0;
	RHI_API RHI_VIRTUAL void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State) = 0;
	RHI_API RHI_VIRTUAL void SetResourceState(RHICommandList* list, EResourceState::Type State) = 0;	
	RHI_API RHI_VIRTUAL void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state) = 0;
	RHI_API RHI_VIRTUAL void* MapReadBack() = 0;
	RHI_API RHI_VIRTUAL void CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List) {};
	RHI_API RHI_VIRTUAL void CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List) {};
	RHI_API RHI_VIRTUAL ~RHIBuffer()
	{}
	size_t GetVertexCount()
	{
		return VertexCount;
	}
	int GetCounterOffset()
	{
		return CounterOffset;
	}
	DeviceContext* GetContext()
	{
		return Context;
	}
	int GetSize(int offset = 0)
	{
		if (offset > 0)
		{
			return TotalByteSize - (StructSize * offset);
		}
		return TotalByteSize;
	}
	RHI_API virtual void Release() override;

	int StructSize = 0;
	int RawStructSize = 0;
	const RHIBufferDesc& GetDesc() const { return Desc; }
protected:
	DeviceContext* Context = nullptr;
	RHIBufferDesc Desc = {};
	size_t VertexCount = 0;
	int CounterOffset = 0;
	int TotalByteSize = 0;
	class RHIUAV* UAV = nullptr;
};
struct RHIScissorRect
{
	RHIScissorRect()
	{

	}
	RHIScissorRect(LONG left, LONG top, LONG right, LONG bottom) :
		Left(left)
		, Top(top)
		, Right(right)
		, Bottom(bottom)
	{}
	LONG Left;
	LONG Top;
	LONG Right;
	LONG Bottom;
};
typedef RHIScissorRect RHICopyRect;
class FrameBuffer;
class RHICommandList : public IRHIResourse
{
public:
	RHI_API RHICommandList(ECommandListType::Type type, DeviceContext* context);
	RHI_API RHI_VIRTUAL ~RHICommandList();
	RHI_API RHI_VIRTUAL void ResetList() = 0;

	RHI_API RHI_VIRTUAL void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) = 0;
	RHI_API RHI_VIRTUAL void SetScissorRect(const RHIScissorRect& rect) = 0;
	RHI_API RHI_VIRTUAL void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) = 0;
	//drawing
	RHI_API RHI_VIRTUAL void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) = 0;
	RHI_API RHI_VIRTUAL void DrawIndexedPrimitive(uint IndexCountPerInstance, uint InstanceCount, uint StartIndexLocation, uint BaseVertexLocation, uint StartInstanceLocation) = 0;
	///Not Const Desc as they hash on demand
	RHI_API RHI_VIRTUAL void SetPipelineStateDesc(const RHIPipeLineStateDesc& Desc) = 0;
	RHI_API RHI_VIRTUAL void SetPipelineStateObject(RHIPipeLineStateObject* Object) = 0;
	//PSO setting functions
	RHI_API RHI_VIRTUAL void SetStencilRef(uint value) = 0;
	//setters
	RHI_API RHI_VIRTUAL void SetVertexBuffer(RHIBuffer* buffer) = 0;
	RHI_API RHI_VIRTUAL void SetIndexBuffer(RHIBuffer* buffer) = 0;
	RHI_API RHI_VIRTUAL void SetConstantBufferView(RHIBuffer * buffer, RHIViewDesc Desc, int Slot) = 0;
	RHI_API RHI_VIRTUAL void SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot) = 0;
	RHI_API RHI_VIRTUAL void SetTexture(BaseTextureRef texture, int slot, const RHIViewDesc & desc) = 0;
	RHI_API RHI_VIRTUAL void SetFrameBufferTexture(FrameBuffer* buffer, int slot, const RHIViewDesc & desc) = 0;
	RHI_API RHI_VIRTUAL void SetBuffer(RHIBuffer* Buffer, int slot, const RHIViewDesc & desc) = 0;
	RHI_API RHI_VIRTUAL void SetUAV(RHIBuffer* buffer, int slot, const RHIViewDesc & view) = 0;
	RHI_API RHI_VIRTUAL void SetUAV(FrameBuffer* buffer, int slot, const RHIViewDesc & view) = 0;
	RHI_API RHI_VIRTUAL void SetUAV(RHITexture* buffer, int slot, const RHIViewDesc & view) = 0;
	RHI_API RHI_VIRTUAL void SetTextureArray(RHITextureArray* array, int slot, const RHIViewDesc& view) = 0;
	RHI_API RHI_VIRTUAL void SetTexture2(RHITexture* t, int slot, const RHIViewDesc& view) = 0;

	//view Creators
	RHI_API void SetUAV(RHIBuffer* uav, int slot);
	RHI_API void SetUAV(FrameBuffer* uav, int slot, int ResourceIndex = 0, int Face = 0, int MipSlice = 0);
	RHI_API void SetUAV(RHIBuffer* uav, std::string slot);
	RHI_API void SetUAV(FrameBuffer* uav, std::string slot, int ResourceIndex = 0, int Face = 0, int MipSlice = 0);
	RHI_API void SetUAV(RHITexture* uav, std::string slot, int ResourceIndex = 0, int Face = 0, int MipSlice = 0);

	RHI_API void SetBuffer(RHIBuffer* Buffer, int slot, int ElementOffset = 0);
	RHI_API void SetBuffer(RHIBuffer* Buffer, std::string  slot, int ElementOffset = 0);

	RHI_API void SetTexture(BaseTextureRef texture, int slot, int mip = 0);
	RHI_API void SetFrameBufferTexture(FrameBuffer* buffer, int slot, int Resourceindex = 0);
	//remove?
	RHI_API	void SetRHIBufferReadOnly(RHIBuffer* buffer, int slot);

	//string setters
	RHI_API void SetConstantBufferView(RHIBuffer * buffer, int offset, std::string Slot);
	RHI_API void SetTexture(BaseTextureRef texture, std::string slot, int mip = 0);
	RHI_API void SetFrameBufferTexture(FrameBuffer* buffer, std::string slot, int Resourceindex = 0);
	RHI_API void SetRHIBufferReadOnly(RHIBuffer* buffer, std::string slot);
	RHI_API void SetTexture2(RHITexture* buffer, std::string slot);
	RHI_API void SetTexture2(RHITexture* buffer, int slot);

	RHI_API RHI_VIRTUAL void ClearFrameBuffer(FrameBuffer* buffer) = 0;
	RHI_API RHI_VIRTUAL void UAVBarrier(FrameBuffer* target) = 0;
	RHI_API RHI_VIRTUAL void UAVBarrier(RHITexture* target) = 0;
	RHI_API RHI_VIRTUAL void UAVBarrier(RHIBuffer* target) = 0;
	RHI_API RHI_VIRTUAL void ClearUAVFloat(RHIBuffer* buffer) = 0;
	RHI_API RHI_VIRTUAL void ClearUAVUint(RHIBuffer* buffer) = 0;
	RHI_API RHI_VIRTUAL void ClearUAVFloat(FrameBuffer* buffer) = 0;
	RHI_API RHI_VIRTUAL void ClearUAVUint(FrameBuffer* buffer) = 0;

	RHI_API RHI_VIRTUAL void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) = 0;
	RHI_API void DispatchSized(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ);
	//Indirect
	RHI_API RHI_VIRTUAL void ExecuteIndirect(int MaxCommandCount, RHIBuffer* ArgumentBuffer, int ArgOffset, RHIBuffer* CountBuffer, int CountBufferOffset) = 0;
	RHI_API RHI_VIRTUAL void SetCommandSigniture(RHICommandSignitureDescription desc);
	RHI_API RHI_VIRTUAL void SetCommandSigniture(RHICommandSigniture* sig) {};
	template<class T>
	void SetSingleRootConstant(int SignitureSlot, T Data)
	{
		SetRootConstant(SignitureSlot, 1, &Data, 0);
	}
	RHI_API RHI_VIRTUAL void SetRootConstant(std::string SignitureSlot, int ValueNum, void* Data, int DataOffset = 0);
	RHI_API RHI_VIRTUAL void SetRootConstant(int SignitureSlot, int ValueNum, void* Data, int DataOffset) = 0;
	//Render Passes
	RHI_API RHI_VIRTUAL void BeginRenderPass(const struct RHIRenderPassDesc& RenderPass);
	RHI_API RHI_VIRTUAL void EndRenderPass();

	RHI_API DeviceContext* GetDevice();
	RHI_API int GetDeviceIndex() const;
	RHI_API void StartTimer(int TimerId);
	RHI_API void EndTimer(int TimerId);
	RHI_API void ResolveTimers();
	//MultiGPU
	RHI_API RHI_VIRTUAL void CopyResourceToSharedMemory(FrameBuffer* Buffer)
	{};
	RHI_API RHI_VIRTUAL void CopyResourceFromSharedMemory(FrameBuffer* Buffer)
	{};
	RHI_API bool IsGraphicsList()const;
	RHI_API bool IsCopyList() const;
	RHI_API bool IsComputeList() const;
	RHI_API bool IsRaytracingList() const;
	RHI_API virtual bool IsOpen()const;
	RHI_API void InsertGPUStallTimer();
	RHI_API void HandleStallTimer();
	//RT
	RHI_API virtual void SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct);
	RHI_API virtual void TraceRays(const RHIRayDispatchDesc& desc);
	RHI_API virtual void SetStateObject(RHIStateObject* Object);

	RHI_API RHI_VIRTUAL void SetDepthBounds(float Min, float Max) = 0;

	//Used to resolve a framebuffer that is using VRX tech (VRS or VRR)
	RHI_API void ResolveVRXFramebuffer(FrameBuffer* Target);

	RHI_API void SetVRSShadingRate(VRX_SHADING_RATE::type Rate);
	void PrepareFramebufferForVRR(RHITexture * RateImage, FrameBuffer * VRRTarget);
	RHI_API void SetVRXShadingRateImage(RHITexture* RateImage);
	RHI_API RHIPipeLineStateObject* GetCurrnetPSO();
	RHI_API ECommandListType::Type GetListType() const;
	//forces any queued barriers to be executed
	RHI_API RHI_VIRTUAL void FlushBarriers();
	RHITexture* GetShadingRateImage() const { return ShadingRateImage; }

	RHI_API RHI_VIRTUAL void CopyResource(RHITexture* Source, RHITexture* Dest) = 0;
	RHI_API RHI_VIRTUAL void CopyResource(RHIBuffer* Source, RHIBuffer* Dest) = 0;
protected:
	RHI_API virtual void SetVRSShadingRateNative(VRX_SHADING_RATE::type Rate);
	RHI_API virtual void SetVRSShadingRateImageNative(RHITexture* Target);

	RHIPipeLineStateObject* CurrentPSO = nullptr;
	bool IsInRenderPass = false;
	DeviceContext * Device = nullptr;
	FrameBuffer * CurrentRenderTarget = nullptr;//todo: multiple!
	ECommandListType::Type ListType = ECommandListType::Graphics;
	RHITexture* ShadingRateImage = nullptr;
};

//Used to Bind Buffers or textures to a single Descriptor heap/set for shader arrays
class RHITextureArray : public IRHIResourse
{
public:
	RHI_API RHITextureArray(DeviceContext* device, int inNumEntries);;
	RHI_API RHI_VIRTUAL ~RHITextureArray()
	{};
	RHI_API RHI_VIRTUAL void AddFrameBufferBind(FrameBuffer* Buffer, int slot) = 0;
	RHI_API RHI_VIRTUAL void BindToShader(RHICommandList* list, int slot) = 0;
	RHI_API RHI_VIRTUAL void SetIndexNull(int TargetIndex, FrameBuffer* Buffer = nullptr) = 0;
	RHI_API RHI_VIRTUAL void Clear() = 0;
	RHI_API RHI_VIRTUAL void SetFrameBufferFormat(const RHIFrameBufferDesc & desc) = 0;
	RHI_API RHI_VIRTUAL void AddTexture(RHITexture* Tex, int Index) = 0;
	RHI_API RHI_VIRTUAL void SetTextureFormat(const RHITextureDesc2& Desc) = 0;

protected:
	int NumEntries = 1;
};

struct RHI_API RHISubPass
{
public:
	FrameBuffer* ColourTargets[MAX_RENDERTARGETS] = { nullptr };
	FrameBuffer* DepthTarget = { nullptr };
	void Validate()
	{};
};

class RHIRenderPass
{
public:
	RHI_API RHIRenderPass(const RHIRenderPassDesc & Desc);
	RHI_API virtual ~RHIRenderPass();
	void AddSubPass(RHISubPass* Pass);
	virtual void Complie();
	RHIRenderPassDesc Desc;
private:
	std::vector<RHISubPass*> SubPasses;
};

typedef  SharedPtr<RHIBuffer> RHIBufferRef;

