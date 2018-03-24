#include "stdafx.h"
#include "D3D12CommandList.h"
#include "D3D12RHI.h"
#include "Rendering/Core/Triangle.h"
#include "../Core/Assets/OBJFileReader.h"
#include "../Core/Utils/StringUtil.h"
#include "D3D12CBV.h"
#include "../EngineGlobals.h"
#include "D3D12Texture.h"
#include "../RHI/Shader.h"
#include "../RHI/BaseTexture.h"
#include "D3D12Framebuffer.h"
D3D12CommandList::D3D12CommandList()
{
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}


D3D12CommandList::~D3D12CommandList()
{
	if (CurrentPipelinestate.m_pipelineState != nullptr)
	{
		CurrentPipelinestate.m_pipelineState->Release();
		CurrentPipelinestate.m_rootSignature->Release();
	}
	if (CurrentGraphicsList != nullptr)
	{
		CurrentGraphicsList->Release();
	}
}

void D3D12CommandList::ResetList()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	/*ThrowIfFailed(CurrentGraphicsList->Close());*/
	ThrowIfFailed(CurrentGraphicsList->Reset(m_commandAllocator, CurrentPipelinestate.m_pipelineState));
	CurrentGraphicsList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
}

void D3D12CommandList::SetRenderTarget(FrameBuffer * target)
{
	if (target == nullptr)
	{
		CurrentRenderTarget->UnBind(CurrentGraphicsList);
	}
	else
	{
		CurrentRenderTarget = (D3D12FrameBuffer*)target;
		CurrentRenderTarget->BindBufferAsRenderTarget(CurrentGraphicsList);
	}
}

void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	CurrentGraphicsList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CurrentGraphicsList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12CommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{}

void D3D12CommandList::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{
	//D3D12RHI::Instance->RenderToScreen(CurrentGraphicsList);
}

void D3D12CommandList::Execute()
{
	ThrowIfFailed(CurrentGraphicsList->Close());
	D3D12RHI::Instance->ExecList(CurrentGraphicsList);
}

void D3D12CommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	CurrentGraphicsList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
}

void D3D12CommandList::CreatePipelineState(Shader * shader)
{
	D3D12Shader* target = (D3D12Shader*)shader->GetShaderProgram();
	ensure((shader->GetShaderParameters().size() > 0));
	ensure((shader->GetVertexFormat().size() > 0));
	D3D12_INPUT_ELEMENT_DESC* desc;
	D3D12Shader::ParseVertexFormat(shader->GetVertexFormat(), &desc, &VertexDesc_ElementCount);
	D3D12Shader::CreateRootSig(CurrentPipelinestate, shader->GetShaderParameters());
	D3D12Shader::CreatePipelineShader(CurrentPipelinestate, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Currentpipestate);

	CreateCommandList();
}

void D3D12CommandList::SetPipelineState(PipeLineState state)
{
	Currentpipestate = state;
}

void D3D12CommandList::CreateCommandList()
{
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentGraphicsList)));
	CurrentGraphicsList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
	ThrowIfFailed(CurrentGraphicsList->Close());
}

void D3D12CommandList::ClearFrameBuffer(FrameBuffer * buffer)
{
	((D3D12FrameBuffer*)buffer)->ClearBuffer(CurrentGraphicsList);
}

void D3D12CommandList::UAVBarrier(RHIUAV * target)
{
	D3D12RHIUAV* dtarget = (D3D12RHIUAV*)target;
	CurrentGraphicsList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(dtarget->m_UAV));
}



void D3D12CommandList::SetScreenBackBufferAsRT()
{
	if (CurrentRenderTarget != nullptr)
	{
		CurrentRenderTarget->UnBind(CurrentGraphicsList);
	}
	D3D12RHI::Instance->SetScreenRenderTaget(CurrentGraphicsList);
	D3D12RHI::Instance->RenderToScreen(CurrentGraphicsList);
}

void D3D12CommandList::ClearScreen()
{
	D3D12RHI::Instance->ClearRenderTarget(CurrentGraphicsList);
}

void D3D12CommandList::SetFrameBufferTexture(FrameBuffer * buffer, int slot)
{

	((D3D12FrameBuffer*)buffer)->BindBufferToTexture(CurrentGraphicsList, slot);
}

void D3D12CommandList::SetTexture(BaseTexture * texture, int slot)
{
	Texsture = (D3D12Texture*)texture;
	if (CurrentGraphicsList != nullptr)
	{
		Texsture->BindToSlot(CurrentGraphicsList, slot);
	}
}

void D3D12CommandList::UpdateConstantBuffer(void * data, int offset)
{
	CurrentConstantBuffer->UpdateConstantBuffer(data, offset);
}

void D3D12CommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Register)
{

	((D3D12Buffer*)buffer)->SetConstantBufferView(offset, CurrentGraphicsList, Register);
}

D3D12Buffer::D3D12Buffer(RHIBuffer::BufferType type) :RHIBuffer(type)
{

}

D3D12Buffer::~D3D12Buffer()
{}

void D3D12Buffer::CreateVertexBufferFromFile(std::string name)
{
	Triangle* mesh;
	int m_numtriangles = importOBJMesh(StringUtils::ConvertStringToWide(name).c_str(), &mesh);
	const int vertexBufferSize = sizeof(OGLVertex)*m_numtriangles * 3;
	CreateVertexBuffer(sizeof(OGLVertex), vertexBufferSize);
	VertexCount = m_numtriangles * 3;
	UpdateVertexBuffer(mesh, vertexBufferSize);
}

void D3D12Buffer::UpdateVertexBuffer(void * data, int length)
{
	UINT8* pVertexDataBegin;
	MapBuffer(reinterpret_cast<void**>(&pVertexDataBegin));

	memcpy(pVertexDataBegin, data, length);
	UnMap();
}

void D3D12Buffer::CreateConstantBuffer(int StructSize, int Elementcount)
{
	CBV = new D3D12CBV();
	CBV->InitCBV(StructSize, Elementcount);
	cpusize = StructSize;
}

void D3D12Buffer::CreateVertexBuffer(int Stride, int ByteSize)
{
	// Create the vertex buffer.
	const int vertexBufferSize = ByteSize;//mazsize

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = Stride;
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBuffer->SetName(StringUtils::ConvertStringToWide("Vertex Buffer").c_str());

}

void D3D12Buffer::UpdateConstantBuffer(void * data, int offset)
{
	CBV->UpdateCBV(data, offset, cpusize);
}

void D3D12Buffer::SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Register)
{
	CBV->SetDescriptorHeaps(list);//D3D12CBV::MPCBV
	CBV->SetGpuView(list, offset, Register);//todo: handle Offset!
}

void D3D12Buffer::MapBuffer(void ** Data)
{
	// Copy the triangle data to the vertex buffer.
//	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, Data));

	//	memcpy(pVertexDataBegin, &BatchedVerts[0], sizeof(UIVertex)*BatchedVerts.size());
		//m_vertexBuffer->Unmap(0, nullptr);

}

void D3D12Buffer::UnMap()
{
	m_vertexBuffer->Unmap(0, nullptr);
}

void D3D12RHITexture::CreateTextureFromFile(std::string name)
{
	tmptext = new D3D12Texture(name);
}

void D3D12RHITexture::BindToSlot(ID3D12GraphicsCommandList * list, int slot)
{
	tmptext->Bind(list);
}
