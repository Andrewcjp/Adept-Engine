#pragma once
class DeviceContext;
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
	enum BufferAccessType
	{
		Static,
		Dynamic
	};
	virtual void CreateVertexBuffer(int Stride, int ByteSize, BufferAccessType Accesstype = BufferAccessType::Static) = 0;
	virtual void CreateIndexBuffer(int Stride, int ByteSize) = 0;
	virtual void CreateConstantBuffer(int StructSize, int Elementcount) = 0;
	virtual void UpdateConstantBuffer(void * data, int offset) = 0;
	virtual void UpdateVertexBuffer(void* data, int length) = 0;
	virtual void UpdateIndexBuffer(void* data, int length) = 0;
	virtual ~RHIBuffer() {}
	int GetVertexCount() { return VertexCount; }
protected:
	int VertexCount = 0;

};
class RHIUAV
{
public:
	RHIUAV()
	{}
};


enum PRIMITIVE_TOPOLOGY_TYPE
{
	PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED = 0,
	PRIMITIVE_TOPOLOGY_TYPE_POINT = 1,
	PRIMITIVE_TOPOLOGY_TYPE_LINE = 2,
	PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE = 3,
	PRIMITIVE_TOPOLOGY_TYPE_PATCH = 4
};
enum COMPARISON_FUNC
{
	COMPARISON_FUNC_NEVER = 1,
	COMPARISON_FUNC_LESS = 2,
	COMPARISON_FUNC_EQUAL = 3,
	COMPARISON_FUNC_LESS_EQUAL = 4,
	COMPARISON_FUNC_GREATER = 5,
	COMPARISON_FUNC_NOT_EQUAL = 6,
	COMPARISON_FUNC_GREATER_EQUAL = 7,
	COMPARISON_FUNC_ALWAYS = 8
};
struct PipeLineState
{
	bool DepthTest = true;
	bool Cull = true;
	bool Blending = false;
	PRIMITIVE_TOPOLOGY_TYPE RasterMode = PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	COMPARISON_FUNC DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS;
	bool DepthWrite = true;
};
class FrameBuffer;
class RHICommandList
{
public:
	RHICommandList();
	virtual ~RHICommandList();
	virtual void ResetList() = 0;
	virtual void SetRenderTarget(FrameBuffer* target) = 0;

	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) = 0;
	virtual void Execute(bool Block = true) = 0;
	virtual void WaitForCompletion() =0;
	//drawing
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) = 0;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) = 0;
	virtual void SetVertexBuffer(RHIBuffer* buffer) = 0;
	virtual void SetIndexBuffer(RHIBuffer* buffer) = 0;
	//If frame buffer is null the screen will be the render target!
	virtual void CreatePipelineState(class Shader* shader, class FrameBuffer* Buffer = nullptr) = 0;
	virtual void SetPipelineState(PipeLineState state) = 0;
	//virtual void SetConstantBuffer(RHIBuffer* buffer) = 0;
	virtual void UpdateConstantBuffer(void * data, int offset) = 0;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) = 0;
	virtual void SetTexture(class BaseTexture* texture, int slot) = 0;
	virtual void SetFrameBufferTexture(class FrameBuffer* buffer, int slot, int Resourceindex = 0) = 0;
	virtual void SetScreenBackBufferAsRT() = 0;
	virtual void ClearScreen() = 0;
	virtual void ClearFrameBuffer(FrameBuffer* buffer) = 0;
	//todo: complete this
	virtual void SetUAVParamter() {};
	virtual void UAVBarrier(RHIUAV* target) = 0;
	virtual void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) = 0;
	DeviceContext* GetDevice();
	int GetDeviceIndex();
protected:
	DeviceContext * Device = nullptr;
	FrameBuffer * CurrentRenderTarget = nullptr;//todo: multiple!
	enum ECommandListType
	{
		Graphics,
		Compute,
		Copy
	};
};

//Used to Bind Buffers or textures to a single Descriptor heap/set for shader arrays
class RHITextureArray
{
public:
	RHITextureArray(DeviceContext* device, int inNumEntries)
	{
		NumEntries = inNumEntries;
	};
	~RHITextureArray() {};
	virtual void AddFrameBufferBind(FrameBuffer* Buffer, int slot) = 0;
	virtual void BindToShader(RHICommandList* list, int slot) = 0;
	virtual void SetIndexNull(int TargetIndex)  = 0;
protected:
	int NumEntries = 1;
	
};