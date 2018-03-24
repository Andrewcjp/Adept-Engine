#pragma once

class RHIBuffer
{
public:
	enum BufferType
	{
		Vertex,
		Index,
		Constant
	};
	BufferType CurrentBufferType;
	RHIBuffer(BufferType type)
	{
		CurrentBufferType = type;
	}
	virtual void CreateVertexBufferFromFile(std::string name) = 0;
	virtual void CreateVertexBuffer(int Stride,int ByteSize) = 0;
	virtual void CreateConstantBuffer(int StructSize, int Elementcount) = 0;
	virtual void UpdateConstantBuffer(void * data, int offset) = 0;
	virtual void UpdateVertexBuffer(void* data, int length) =0;
	virtual ~RHIBuffer() {}
	int GetVertexCount() { return VertexCount; }
protected:
	int VertexCount = 0;

};
class RHIUAV
{
public:
	RHIUAV(class BaseTexture* Target)
	{}	
};
class RHITexture
{
public :
	enum TextureType
	{
		Standard,
		Cubemap,
		Normal
	};
	RHITexture(TextureType type = TextureType::Standard) {
		CurrentType = type;
	}
	virtual ~RHITexture() {}
	virtual void CreateTextureFromFile(std::string name) = 0;

protected:
	TextureType CurrentType;
};

struct PipeLineState
{
	bool DepthTest = true;
	bool Cull = true;
};
class FrameBuffer;
class RHICommandList
{
public:
	RHICommandList();
	~RHICommandList();
	virtual void ResetList() = 0;
	virtual void SetRenderTarget(FrameBuffer* target) = 0;

	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) = 0;
	virtual void Execute() = 0;
	//drawing
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) = 0;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) = 0;
	virtual void SetVertexBuffer(RHIBuffer* buffer) = 0;
	virtual void CreatePipelineState(class Shader* shader) = 0;
	virtual void SetPipelineState(PipeLineState state) = 0;
	//virtual void SetConstantBuffer(RHIBuffer* buffer) = 0;
	virtual void UpdateConstantBuffer(void * data, int offset) = 0;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) = 0;
	virtual void SetTexture(class BaseTexture* texture, int slot) = 0;
	virtual void SetFrameBufferTexture(class FrameBuffer* buffer, int slot) = 0;
	virtual void SetScreenBackBufferAsRT() = 0;
	virtual void ClearScreen() = 0;
	virtual void ClearFrameBuffer(FrameBuffer* buffer) = 0;
	//todo:
	virtual void SetUAVParamter() {};
	virtual void UAVBarrier(RHIUAV* target) = 0;
	//Set Shader Params?
	//For validiotn on D3d12 Side But needed for opengl Using reflections!

protected:
	FrameBuffer * CurrentRenderTarget = nullptr;//todo: multiple!
};

