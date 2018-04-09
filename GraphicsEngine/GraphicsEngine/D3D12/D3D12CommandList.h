#pragma once
#include "../RHI/RHICommandList.h"
#include <d3d12.h>
#include "D3D12Shader.h"
class D3D12CommandList : public RHICommandList
{
public:
	D3D12CommandList();
	~D3D12CommandList();

	// Inherited via RHICommandList
	virtual void ResetList() override;
	virtual void SetRenderTarget(FrameBuffer * target) override;
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;
	virtual void Execute() override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void SetIndexBuffer(RHIBuffer* buffer)override;
	virtual void CreatePipelineState(class Shader * shader, class FrameBuffer* Buffer = nullptr) override;
	virtual void SetPipelineState(PipeLineState state) override;

	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetTexture(class BaseTexture * texture, int slot) override;
	// Inherited via RHICommandList
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot) override;
	virtual void SetScreenBackBufferAsRT() override;
	virtual void ClearScreen() override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(class RHIUAV* target) override;




private:
	void CreateCommandList();
	ID3D12GraphicsCommandList * CurrentGraphicsList = nullptr;
	bool IsOpen = false;
	//struct PiplineShader
	//{
	//	ID3D12PipelineState* m_pipelineState;
	//	ID3D12RootSignature* m_rootSignature;
	//};
	D3D12Shader::PiplineShader				CurrentPipelinestate;
	ID3D12CommandAllocator* m_commandAllocator;
	ID3DBlob*					m_vsBlob;
	ID3DBlob*					m_fsBlob;
	ID3DBlob*					m_csBlob;
	D3D12_INPUT_ELEMENT_DESC* VertexDesc = nullptr;
	int VertexDesc_ElementCount = 0;
	class D3D12Buffer* CurrentConstantBuffer = nullptr;
	class D3D12Texture* Texsture = nullptr;
	class D3D12FrameBuffer* CurrentRenderTarget = nullptr;
	PipeLineState Currentpipestate;
	// Inherited via RHICommandList

};

class D3D12Buffer : public RHIBuffer
{
public:
	D3D12Buffer(RHIBuffer::BufferType type);
	virtual ~D3D12Buffer();
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	virtual void CreateVertexBufferFromFile(std::string name) override;
	virtual void CreateConstantBuffer(int StructSize, int Elementcount) override;
	virtual void CreateVertexBuffer(int Stride, int ByteSize, BufferAccessType Accesstype = BufferAccessType::Static) override;
	void CreateStaticBuffer(int Stride, int ByteSize);
	void CreateDynamicBuffer(int Stride, int ByteSize);
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Register);
	virtual void UpdateIndexBuffer(void* data, int length) override;
	virtual void CreateIndexBuffer(int Stride, int ByteSize) override;
	void MapBuffer(void** Data);
	void UnMap();
	virtual void UpdateVertexBuffer(void* data, int length) override;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
private:
	BufferAccessType BufferAccesstype;
	ID3D12Resource * m_vertexBuffer;
	ID3D12Resource * m_indexBuffer;
	ID3D12Resource * m_UploadBuffer;
	class D3D12CBV* CBV = nullptr;
	int cpusize = 0;
	bool UploadComplete = false;
	int vertexBufferSize = 0;
};
class D3D12RHIUAV : public RHIUAV
{
public:
	D3D12RHIUAV(class BaseTexture* Target) :RHIUAV(Target)
	{
		TargetTexture = (D3D12Texture*)Target;
	}

	ID3D12Resource * m_UAV;
	class D3D12Texture * TargetTexture;
};
class D3D12RHITexture : public RHITexture
{

public:
	D3D12RHITexture(RHITexture::TextureType type = RHITexture::TextureType::Standard) :RHITexture(type)
	{

	}
	~D3D12RHITexture() {};
	virtual void CreateTextureFromFile(std::string name) override;
	void BindToSlot(ID3D12GraphicsCommandList* list, int slot);
private:
	class	D3D12Texture * tmptext = nullptr;
};