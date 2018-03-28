#pragma once
#include "../RHI/RHICommandList.h"
#include "../RHI/Shader.h"
class VKanCommandlist :
	public RHICommandList
{
public:
	VKanCommandlist();
	~VKanCommandlist();

	// Inherited via RHICommandList
	virtual void ResetList() override;
	virtual void SetRenderTarget(FrameBuffer * target) override;
	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;
	virtual void Execute() override;
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void CreatePipelineState(Shader * shader) override;
	virtual void SetPipelineState(PipeLineState state) override;
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetTexture(BaseTexture * texture, int slot) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot) override;
	virtual void SetScreenBackBufferAsRT() override;
	virtual void ClearScreen() override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(RHIUAV * target) override;
};

class VKanBuffer : public RHIBuffer
{
public:
	VKanBuffer(RHIBuffer::BufferType type) :RHIBuffer(type) {}
	virtual ~VKanBuffer() {};
	// Inherited via RHIBuffer
	virtual void CreateVertexBufferFromFile(std::string name) override;
	virtual void CreateVertexBuffer(int Stride, int ByteSize) override;
	virtual void CreateConstantBuffer(int StructSize, int Elementcount) override;
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void UpdateVertexBuffer(void * data, int length) override;
};