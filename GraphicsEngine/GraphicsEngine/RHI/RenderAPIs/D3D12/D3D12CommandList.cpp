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
#include "../RHI/DeviceContext.h"

D3D12CommandList::D3D12CommandList(DeviceContext * inDevice)
{
	Device = inDevice;
	ThrowIfFailed(Device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
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
	IsOpen = true;
	ThrowIfFailed(CurrentGraphicsList->Reset(m_commandAllocator, CurrentPipelinestate.m_pipelineState));
	CurrentGraphicsList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
}

void D3D12CommandList::SetRenderTarget(FrameBuffer * target)
{
	if (target == nullptr)
	{
		CurrentRenderTarget->UnBind(CurrentGraphicsList);
		CurrentRenderTarget = nullptr;
	}
	else
	{
		CurrentRenderTarget = (D3D12FrameBuffer*)target;
		CurrentRenderTarget->BindBufferAsRenderTarget(CurrentGraphicsList);
	}
}

void D3D12CommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	ensure(IsOpen);
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

	IsOpen = false;
}

void D3D12CommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	CurrentGraphicsList->IASetVertexBuffers(0, 1, &dbuffer->m_vertexBufferView);
}

void D3D12CommandList::SetIndexBuffer(RHIBuffer * buffer)
{
	D3D12Buffer* dbuffer = (D3D12Buffer*)buffer;
	CurrentGraphicsList->IASetIndexBuffer(&dbuffer->m_IndexBufferView);
}

void D3D12CommandList::CreatePipelineState(Shader * shader, class FrameBuffer* Buffer)
{
	if (CurrentPipelinestate.m_pipelineState != nullptr )
	{
		CurrentPipelinestate.m_pipelineState->Release();
	}
	if (CurrentPipelinestate.m_rootSignature != nullptr)
	{
		CurrentPipelinestate.m_rootSignature->Release();
	}
	D3D12Shader* target = (D3D12Shader*)shader->GetShaderProgram();
	D3D12FrameBuffer* dbuffer = (D3D12FrameBuffer*)Buffer;
	ensure((shader->GetShaderParameters().size() > 0));
	ensure((shader->GetVertexFormat().size() > 0));
	D3D12_INPUT_ELEMENT_DESC* desc;
	D3D12Shader::ParseVertexFormat(shader->GetVertexFormat(), &desc, &VertexDesc_ElementCount);
	D3D12Shader::CreateRootSig(CurrentPipelinestate, shader->GetShaderParameters());

	D3D12Shader::PipeRenderTargetDesc PRTD = {};
	if (Buffer == nullptr)
	{
		PRTD.NumRenderTargets = 1;
		PRTD.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		PRTD.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	}
	else
	{
		PRTD = dbuffer->GetPiplineRenderDesc();
	}
	D3D12Shader::CreatePipelineShader(CurrentPipelinestate, desc, VertexDesc_ElementCount, target->GetShaderBlobs(), Currentpipestate, PRTD);
	if (CurrentGraphicsList == nullptr)
	{
		//todo: ensure a gaphics shader is not used a compute piplne!
		CreateCommandList();
	}
}

void D3D12CommandList::SetPipelineState(PipeLineState state)
{
	Currentpipestate = state;
}



void D3D12CommandList::CreateCommandList(ECommandListType listype)
{
	if (listype == ECommandListType::Graphics)
	{
		ThrowIfFailed(Device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentGraphicsList)));
		CurrentGraphicsList->SetGraphicsRootSignature(CurrentPipelinestate.m_rootSignature);
		ThrowIfFailed(CurrentGraphicsList->Close());
	}
	else if (listype == ECommandListType::Compute)
	{
		//todo: aloccators?
		ThrowIfFailed(Device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_commandAllocator, CurrentPipelinestate.m_pipelineState, IID_PPV_ARGS(&CurrentGraphicsList)));
		CurrentGraphicsList->SetComputeRootSignature(CurrentPipelinestate.m_rootSignature);
		ThrowIfFailed(CurrentGraphicsList->Close());
	}
	else if (listype == ECommandListType::Copy)
	{

	}
}

void D3D12CommandList::ClearFrameBuffer(FrameBuffer * buffer)
{
	((D3D12FrameBuffer*)buffer)->ClearBuffer(CurrentGraphicsList);
}
//todo: move to gpuresrouce!
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
		CurrentRenderTarget = nullptr;
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



D3D12Buffer::D3D12Buffer(RHIBuffer::BufferType type, DeviceContext * inDevice) :RHIBuffer(type)
{
	if (inDevice == nullptr)
	{
		Device = D3D12RHI::GetDefaultDevice();
	}
}

D3D12Buffer::~D3D12Buffer()
{}

void D3D12Buffer::CreateVertexBufferFromFile(std::string name)
{
	Triangle* mesh;
	int m_numtriangles = importOBJMesh(StringUtils::ConvertStringToWide(name).c_str(), &mesh);
	const int vertexBufferSize = sizeof(OGLVertex)*m_numtriangles * 3;
	CreateVertexBuffer(sizeof(OGLVertex), vertexBufferSize,RHIBuffer::BufferAccessType::Dynamic);
	VertexCount = m_numtriangles * 3;
	UpdateVertexBuffer(mesh, vertexBufferSize);
}



void D3D12Buffer::CreateConstantBuffer(int StructSize, int Elementcount)
{
	CBV = new D3D12CBV();
	CBV->InitCBV(StructSize, Elementcount);
	cpusize = StructSize;
}

void D3D12Buffer::CreateVertexBuffer(int Stride, int ByteSize, BufferAccessType Accesstype)
{

	BufferAccesstype = Accesstype;
	/*BufferAccesstype = BufferAccessType::Dynamic;*/
	if (BufferAccesstype == BufferAccessType::Dynamic)
	{
		CreateDynamicBuffer(Stride, ByteSize);
	}
	else if (BufferAccesstype == BufferAccessType::Static)
	{
		CreateStaticBuffer(Stride, ByteSize);
	}

}
void D3D12Buffer::UpdateVertexBuffer(void * data, int length)
{
	if (BufferAccesstype == BufferAccessType::Dynamic)
	{
		UINT8* pVertexDataBegin;
		MapBuffer(reinterpret_cast<void**>(&pVertexDataBegin));

		memcpy(pVertexDataBegin, data, length);
		UnMap();
	}
	else
	{
		ensure(!UploadComplete && "Uploading More than once to a GPU only buffer is not allowed!");
		// store vertex buffer in upload heap
		D3D12_SUBRESOURCE_DATA Data = {};
		Data.pData = reinterpret_cast<BYTE*>(data); // pointer to our index array
		Data.RowPitch = vertexBufferSize; // size of all our index buffer
		Data.SlicePitch = vertexBufferSize; // also the size of our index buffer

											// we are now creating a command with the command list to copy the data from
											// the upload heap to the default heap
		UpdateSubresources(D3D12RHI::Instance->m_SetupCommandList, m_vertexBuffer, m_UploadBuffer, 0, 0, 1, &Data);

		// transition the vertex buffer data from copy destination state to vertex buffer state
		D3D12RHI::Instance->m_SetupCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		UploadComplete = true;
		D3D12RHI::Instance->AddUploadToUsed(m_UploadBuffer);
	}
}
void D3D12Buffer::CreateStaticBuffer(int Stride, int ByteSize)
{
	const int vertexBufferSize = ByteSize;//mazsize
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&m_vertexBuffer)));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	m_vertexBuffer->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)));
	m_UploadBuffer->SetName(L"Index Buffer Upload Resource Heap");
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = Stride;
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBuffer->SetName(StringUtils::ConvertStringToWide("Vertex Buffer").c_str());
}
void D3D12Buffer::CreateDynamicBuffer(int Stride, int ByteSize)
{
	//This Vertex Buffer Will Have Data Changed Every frame so no need to transiton to only gpu.
	// Create the vertex buffer.
	vertexBufferSize = ByteSize;//mazsize

	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
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

void D3D12Buffer::UpdateIndexBuffer(void * data, int length)
{
	UINT8* pIndexDataBegin;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, data, length);
	m_indexBuffer->Unmap(0, nullptr);
}

void D3D12Buffer::CreateIndexBuffer(int Stride, int ByteSize)
{
	const int vertexBufferSize = ByteSize;

	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)));

	// Initialize the vertex buffer view.
	m_IndexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = vertexBufferSize;
	m_indexBuffer->SetName(StringUtils::ConvertStringToWide("Vertex Buffer").c_str());
}

void D3D12Buffer::MapBuffer(void ** Data)
{
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, Data));
}

void D3D12Buffer::UnMap()
{
	m_vertexBuffer->Unmap(0, nullptr);
}

