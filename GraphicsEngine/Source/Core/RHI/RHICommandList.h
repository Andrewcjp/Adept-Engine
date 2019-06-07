#pragma once
#include "RHITypes.h"
#include "BaseTexture.h"
class DeviceContext;
class HighLevelAccelerationStructure;
struct RHIBufferDesc
{
	int ElementCount = 0;
	int Stride = 0;
	int CounterSize = 0;
	EBufferAccessType::Type Accesstype;
	bool AllowUnorderedAccess = false;
	bool CreateSRV = false;
	bool CreateUAV = false;
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
	RHI_API RHI_VIRTUAL void UpdateVertexBuffer(void* data, size_t length) = 0;
	RHI_API RHI_VIRTUAL void UpdateIndexBuffer(void* data, size_t length) = 0;
	RHI_API RHI_VIRTUAL void BindBufferReadOnly(class RHICommandList* list, int RSSlot) = 0;
	RHI_API RHI_VIRTUAL void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State) = 0;
	RHI_API RHI_VIRTUAL void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state) = 0;
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
	class RHIUAV* GetUAV();
	DeviceContext* GetContext()
	{
		return Context;
	}
protected:
	DeviceContext* Context = nullptr;
	RHIBufferDesc Desc = {};
	size_t VertexCount = 0;
	int CounterOffset = 0;
	int TotalByteSize = 0;
	class RHIUAV* UAV = nullptr;
};

class RHI_API RHIUAV : public IRHIResourse
{
public:
	RHIUAV();
	RHI_VIRTUAL ~RHIUAV()
	{};
	RHI_VIRTUAL void Bind(class RHICommandList* list, int slot) = 0;
	RHI_VIRTUAL void CreateUAVFromFrameBuffer(class FrameBuffer* target, int mip = 0) = 0;
	RHI_VIRTUAL void CreateUAVFromTexture(class BaseTexture* target) = 0;
	RHI_VIRTUAL void CreateUAVFromRHIBuffer(class RHIBuffer* target) = 0;
};

class FrameBuffer;
class RHI_API RHICommandList : public IRHIResourse
{
public:
	RHICommandList(ECommandListType::Type type, DeviceContext* context);
	RHI_VIRTUAL ~RHICommandList();
	RHI_VIRTUAL void ResetList() = 0;
	[[deprecated("Use RenderPass API")]]
	RHI_VIRTUAL void SetRenderTarget(FrameBuffer* target, int SubResourceIndex = 0){}
	RHI_VIRTUAL void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) = 0;
	RHI_VIRTUAL void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) = 0;
	//drawing
	RHI_VIRTUAL void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) = 0;
	RHI_VIRTUAL void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) = 0;
	///Not Const Desc as they hash on demand
	RHI_VIRTUAL void SetPipelineStateDesc(RHIPipeLineStateDesc& Desc) = 0;
	RHI_VIRTUAL void SetPipelineStateObject(RHIPipeLineStateObject* Object) = 0;
	//setters
	RHI_VIRTUAL void SetVertexBuffer(RHIBuffer* buffer) = 0;
	RHI_VIRTUAL void SetIndexBuffer(RHIBuffer* buffer) = 0;
	RHI_VIRTUAL void SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot) = 0;
	RHI_VIRTUAL void SetTexture(BaseTextureRef texture, int slot) = 0;
	RHI_VIRTUAL void SetFrameBufferTexture(class FrameBuffer* buffer, int slot, int Resourceindex = 0) = 0;
	void SetRHIBufferReadOnly(RHIBuffer* buffer, int slot);
	void SetUAV(RHIUAV* uav, int slot);
	//string setters
	void SetConstantBufferView(RHIBuffer * buffer, int offset, std::string Slot);
	void SetTexture(BaseTextureRef texture, std::string slot);
	void SetFrameBufferTexture(class FrameBuffer* buffer, std::string slot, int Resourceindex = 0);
	void SetRHIBufferReadOnly(RHIBuffer* buffer, std::string slot);
	void SetUAV(RHIUAV* uav, std::string slot);

	RHI_VIRTUAL void ClearFrameBuffer(FrameBuffer* buffer) = 0;
	RHI_VIRTUAL void UAVBarrier(RHIUAV* target) = 0;
	RHI_VIRTUAL void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) = 0;
	//Indirect
	RHI_VIRTUAL void SetUpCommandSigniture(int commandSize, bool Dispatch) = 0;
	RHI_VIRTUAL void ExecuteIndiect(int MaxCommandCount, RHIBuffer* ArgumentBuffer, int ArgOffset, RHIBuffer* CountBuffer, int CountBufferOffset) = 0;
	template<class T>
	void SetSingleRootConstant(int SignitureSlot, T Data)
	{
		SetRootConstant(SignitureSlot, 1, &Data, 0);
	}
	RHI_VIRTUAL void SetRootConstant(int SignitureSlot, int ValueNum, void* Data, int DataOffset) = 0;
	//Render Passes
	RHI_VIRTUAL void BeginRenderPass(struct RHIRenderPassDesc& RenderPass);
	RHI_VIRTUAL void EndRenderPass();

	DeviceContext* GetDevice();
	int GetDeviceIndex() const;
	void StartTimer(int TimerId);
	void EndTimer(int TimerId);
	void ResolveTimers();
	//MultiGPU
	RHI_VIRTUAL void CopyResourceToSharedMemory(FrameBuffer* Buffer)
	{};
	RHI_VIRTUAL void CopyResourceFromSharedMemory(FrameBuffer* Buffer)
	{};
	bool IsGraphicsList()const;
	bool IsCopyList() const;
	bool IsComputeList() const;
	bool IsRaytracingList() const;
	void InsertGPUStallTimer();
	void HandleStallTimer();
	//RT
	virtual void SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct) = 0;
	virtual void TraceRays(const RHIRayDispatchDesc& desc) = 0;
	virtual void SetStateObject(RHIStateObject* Object) = 0;
protected:
	RHIPipeLineStateObject* CurrentPSO = nullptr;
	bool IsInRenderPass = false;
	DeviceContext * Device = nullptr;
	FrameBuffer * CurrentRenderTarget = nullptr;//todo: multiple!
	ECommandListType::Type ListType = ECommandListType::Graphics;
};

//Used to Bind Buffers or textures to a single Descriptor heap/set for shader arrays
class RHI_API RHITextureArray : public IRHIResourse
{
public:
	RHITextureArray(DeviceContext* device, int inNumEntries);;
	RHI_VIRTUAL ~RHITextureArray()
	{};
	RHI_VIRTUAL void AddFrameBufferBind(FrameBuffer* Buffer, int slot) = 0;
	RHI_VIRTUAL void BindToShader(RHICommandList* list, int slot) = 0;
	RHI_VIRTUAL void SetIndexNull(int TargetIndex, FrameBuffer* Buffer = nullptr) = 0;
	RHI_VIRTUAL void Clear() = 0;
	RHI_VIRTUAL void SetFrameBufferFormat(RHIFrameBufferDesc & desc) = 0;
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
	RHI_API RHIRenderPass(RHIRenderPassDesc & Desc);
	RHI_API virtual ~RHIRenderPass();
	void AddSubPass(RHISubPass* Pass);
	virtual void Complie();
	RHIRenderPassDesc Desc;
private:
	std::vector<RHISubPass*> SubPasses;
};

typedef  SharedPtr<RHIBuffer> RHIBufferRef;

