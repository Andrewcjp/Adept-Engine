#include "stdafx.h"
#include "VKanCommandlist.h"


VKanCommandlist::VKanCommandlist()
{}


VKanCommandlist::~VKanCommandlist()
{}

void VKanCommandlist::ResetList()
{}

void VKanCommandlist::SetRenderTarget(FrameBuffer * target)
{}

void VKanCommandlist::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{}

void VKanCommandlist::Execute()
{}

void VKanCommandlist::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{}

void VKanCommandlist::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{}

void VKanCommandlist::SetVertexBuffer(RHIBuffer * buffer)
{}

void VKanCommandlist::CreatePipelineState(Shader * shader, class FrameBuffer* Buffer)
{}

void VKanCommandlist::SetPipelineState(PipeLineState state)
{}

void VKanCommandlist::UpdateConstantBuffer(void * data, int offset)
{}

void VKanCommandlist::SetConstantBufferView(RHIBuffer * buffer, int offset, int Register)
{}

void VKanCommandlist::SetTexture(BaseTexture * texture, int slot)
{}

void VKanCommandlist::SetFrameBufferTexture(FrameBuffer * buffer, int slot)
{}

void VKanCommandlist::SetScreenBackBufferAsRT()
{}

void VKanCommandlist::ClearScreen()
{}

void VKanCommandlist::ClearFrameBuffer(FrameBuffer * buffer)
{}

void VKanCommandlist::UAVBarrier(RHIUAV * target)
{}

void VKanCommandlist::SetIndexBuffer(RHIBuffer * buffer)
{}

void VKanCommandlist::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{}



void VKanBuffer::CreateConstantBuffer(int StructSize, int Elementcount)
{}

void VKanBuffer::UpdateConstantBuffer(void * data, int offset)
{}

void VKanBuffer::UpdateVertexBuffer(void * data, int length)
{}

void VKanBuffer::CreateVertexBuffer(int Stride, int ByteSize, BufferAccessType Accesstype)
{}

void VKanBuffer::CreateIndexBuffer(int Stride, int ByteSize)
{}

void VKanBuffer::UpdateIndexBuffer(void * data, int length)
{}
