#pragma once
#include "../RHI/RHICommandList.h"
class OGLCommandList :
	public RHICommandList
{
public:
	OGLCommandList();
	virtual ~OGLCommandList();

	// Inherited via RHICommandList
	virtual void ResetList() override;
	virtual void SetRenderTarget(FrameBuffer * target) override;
	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;
	virtual void Execute() override;
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void CreatePipelineState(class Shader * shader) override;
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetTexture(BaseTexture * texture, int slot) override;
	virtual void SetScreenBackBufferAsRT() override;
	virtual void ClearScreen() override;

	// Inherited via RHICommandList
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot) override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(RHIUAV * target) override;
	virtual void SetPipelineState(PipeLineState state) override;
private:
	PipeLineState state;
	class OGLShaderProgram* shaderprog = nullptr;
};

class OGLBuffer : public RHIBuffer
{
public:
	OGLBuffer(RHIBuffer::BufferType type);
	virtual ~OGLBuffer();

	// Inherited via RHIBuffer
	virtual void CreateVertexBufferFromFile(std::string name) override;
	virtual void CreateConstantBuffer(int StructSize, int Elementcount) override;
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void CreateVertexBuffer(int Stride, int ByteSize) override;
	virtual void UpdateVertexBuffer(void * data, int length) override;

	GLuint m_vao = 0;
	GLuint m_vbo_verts = 0;
	int VertexCount = 0;
	GLuint ubo = 0;
	int mStructSize = 0;


};