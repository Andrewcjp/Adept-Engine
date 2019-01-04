#pragma once

#include "RHITypes.h"
class DeviceContext;

namespace EBufferAccessType
{
	enum Type
	{
		Static,
		Dynamic,
		GPUOnly
	};
};
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
class RHI_API RHIBuffer : public IRHIResourse
{
public:

	ERHIBufferType::Type CurrentBufferType;
	RHIBuffer(ERHIBufferType::Type type);
	RHI_VIRTUAL void CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype = EBufferAccessType::Static) = 0;
	RHI_VIRTUAL void CreateBuffer(RHIBufferDesc Desc) = 0;
	RHI_VIRTUAL void CreateIndexBuffer(int Stride, int ByteSize) = 0;
	RHI_VIRTUAL void CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices = false) = 0;
	RHI_VIRTUAL void UpdateConstantBuffer(void * data, int offset) = 0;
	RHI_VIRTUAL void UpdateVertexBuffer(void* data, size_t length) = 0;
	RHI_VIRTUAL void UpdateIndexBuffer(void* data, size_t length) = 0;
	RHI_VIRTUAL void BindBufferReadOnly(class RHICommandList* list, int RSSlot) = 0;
	RHI_VIRTUAL void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State) = 0;
	RHI_VIRTUAL void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state) = 0;
	RHI_VIRTUAL ~RHIBuffer()
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
protected:
	RHIBufferDesc Desc;
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
	RHI_VIRTUAL void CreateUAVFromFrameBuffer(class FrameBuffer* target) = 0;
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
	RHI_VIRTUAL void SetRenderTarget(FrameBuffer* target, int SubResourceIndex = 0) = 0;
	RHI_VIRTUAL void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) = 0;
	RHI_VIRTUAL void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) = 0;
	//drawing
	RHI_VIRTUAL void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) = 0;
	RHI_VIRTUAL void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) = 0;
	RHI_VIRTUAL void SetVertexBuffer(RHIBuffer* buffer) = 0;
	RHI_VIRTUAL void SetIndexBuffer(RHIBuffer* buffer) = 0;
	//If frame buffer is null the screen will be the render target!
	RHI_VIRTUAL void CreatePipelineState(class Shader* shader, class FrameBuffer* Buffer = nullptr) = 0;
	//uses the Cached PiplineStates
	RHI_VIRTUAL void SetPipelineStateObject(class Shader* shader, class FrameBuffer* Buffer = nullptr) = 0;
	RHI_VIRTUAL void SetPipelineState(PipeLineState state) = 0;
	RHI_VIRTUAL void UpdateConstantBuffer(void * data, int offset) = 0;
	RHI_VIRTUAL void SetConstantBufferView(RHIBuffer * buffer, int offset, int Slot) = 0;
	RHI_VIRTUAL void SetTexture(class BaseTexture* texture, int slot) = 0;
	RHI_VIRTUAL void SetFrameBufferTexture(class FrameBuffer* buffer, int slot, int Resourceindex = 0) = 0;
	RHI_VIRTUAL void SetScreenBackBufferAsRT() = 0;
	RHI_VIRTUAL void ClearScreen() = 0;
	RHI_VIRTUAL void ClearFrameBuffer(FrameBuffer* buffer) = 0;
	//todo: complete this
	RHI_VIRTUAL void UAVBarrier(RHIUAV* target) = 0;
	RHI_VIRTUAL void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) = 0;
	//Indirect
	RHI_VIRTUAL void SetUpCommandSigniture(int commandSize, bool Dispatch) = 0;
	RHI_VIRTUAL void ExecuteIndiect(int MaxCommandCount, RHIBuffer* ArgumentBuffer, int ArgOffset, RHIBuffer* CountBuffer, int CountBufferOffset) = 0;
	RHI_VIRTUAL void SetRootConstant(int SignitureSlot, int ValueNum, void* Data, int DataOffset) = 0;
	DeviceContext* GetDevice();
	int GetDeviceIndex();
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
	void InsertGPUStallTimer();
	void HandleStallTimer();
protected:
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
	RHI_VIRTUAL void SetIndexNull(int TargetIndex) = 0;
protected:
	int NumEntries = 1;

};