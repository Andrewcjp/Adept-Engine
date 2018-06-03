#include "stdafx.h"
#include "D3D12Framebuffer.h"
#include "D3D12RHI.h"
#include "GPUResource.h"
#include "../RHI/DeviceContext.h"
#include <algorithm>
#include "DescriptorHeap.h"
#define CUBE_SIDES 6

void D3D12FrameBuffer::CreateCubeDepth()
{
	RTVHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	ID3D12Resource * NewRenderTarget;
	RTVformat = DXGI_FORMAT_D32_FLOAT;
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = RTVformat;
	depthOptimizedClearValue.Color[0] = CubeDepthclearColor[0];
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(RTVformat, m_width, m_height, 6, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&NewRenderTarget)
	));

	RenderTarget[0] = new GPUResource(NewRenderTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	D3D12_DEPTH_STENCIL_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = RTVformat;
	renderTargetViewDesc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	renderTargetViewDesc.Texture2DArray.ArraySize = CUBE_SIDES;
	renderTargetViewDesc.Texture2DArray.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
	CurrentDevice->GetDevice()->CreateDepthStencilView(NewRenderTarget, &renderTargetViewDesc, RTVHeap->GetCPUAddress(0));

	CreateSRVHeap(1);
	CreateSRVInHeap(0, SrvHeap);
}

void D3D12FrameBuffer::CreateSRVHeap(int Num)
{
	if (SrvHeap != nullptr)
	{
		SrvHeap->Release();
		delete SrvHeap;
	}
	SrvHeap = new DescriptorHeap(CurrentDevice, Num, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SrvHeap->SetName(L"Framebuffer Heap");
	if (NullHeap == nullptr)
	{
		NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &GetSrvDesc(), NullHeap->GetCPUAddress(0));
	}
}

void D3D12FrameBuffer::CreateSRVInHeap(int index, DescriptorHeap* targetheap)
{
	if (m_ftype == FrameBufferType::CubeDepth)
	{
		CurrentDevice->GetDevice()->CreateShaderResourceView(RenderTarget[0]->GetResource(), &GetSrvDesc(), targetheap->GetCPUAddress(index));
	}
	else if (m_ftype == FrameBufferType::Depth)
	{
		CurrentDevice->GetDevice()->CreateShaderResourceView(DepthStencil->GetResource(), &GetSrvDesc(), targetheap->GetCPUAddress(index));
	}
}

D3D12_SHADER_RESOURCE_VIEW_DESC D3D12FrameBuffer::GetSrvDesc()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	if (m_ftype == FrameBufferType::CubeDepth)
	{
		shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		shadowSrvDesc.TextureCube.MipLevels = 1;
		shadowSrvDesc.TextureCube.MostDetailedMip = 0;
		shadowSrvDesc.TextureCube.ResourceMinLODClamp = 0;
	}
	else if (m_ftype == FrameBufferType::Depth)
	{
		shadowSrvDesc.Format = DefaultDepthReadformat;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Texture2D.MostDetailedMip = 0;
		shadowSrvDesc.Texture2D.ResourceMinLODClamp = 0;
	}
	else
	{
		NoImpl();
	}
	return shadowSrvDesc;
}

void D3D12FrameBuffer::CreateColour(int Index)
{
	if (Index == 2)
	{
		RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	ID3D12Resource * NewRenderTarget;
	if (RTVHeap == nullptr)
	{
		RTVHeap = new DescriptorHeap(CurrentDevice, RenderTargetCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	}
	D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = RTVformat;
	renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = renderTargetViewDesc.Format;
	depthOptimizedClearValue.Color[0] = BufferClearColour.r;
	depthOptimizedClearValue.Color[1] = BufferClearColour.g;
	depthOptimizedClearValue.Color[2] = BufferClearColour.b;
	depthOptimizedClearValue.Color[3] = BufferClearColour.a;

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(renderTargetViewDesc.Format, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&NewRenderTarget)
	));

	RenderTarget[Index] = new GPUResource(NewRenderTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CurrentDevice->GetDevice()->CreateRenderTargetView(NewRenderTarget, &renderTargetViewDesc, RTVHeap->GetCPUAddress(Index));
	CreateSRV();
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = renderTargetViewDesc.Format;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	CurrentDevice->GetDevice()->CreateShaderResourceView(NewRenderTarget, &SrvDesc, SrvHeap->GetCPUAddress(Index));

}

void D3D12FrameBuffer::CreateSRV()
{
	if (SrvHeap == nullptr)
	{
		SrvHeap = new DescriptorHeap(CurrentDevice, std::max(RenderTargetCount, 1), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		SrvHeap->SetName(L"DSV Heap");
		if (m_ftype == Depth)
		{
			NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}
}

void D3D12FrameBuffer::CreateDepth()
{
	Depthformat = DefaultDepthformat;
	if (DSVHeap == nullptr)
	{
		DSVHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = Depthformat;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = Depthformat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;
	ID3D12Resource* m_depthStencil = nullptr;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(Depthformat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));
	DepthStencil = new GPUResource(m_depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	//NAME_D3D12_OBJECT(m_depthStencil);
	CurrentDevice->GetDevice()->CreateDepthStencilView(DepthStencil->GetResource(), &depthStencilDesc, DSVHeap->GetCPUAddress(0));
	DepthStencil->SetName(L"FrameBuffer Stencil");

	CreateSRV();
	if (m_ftype == Depth)
	{

		//todo look at usage of heaps?	
		//todo: resize might leak descritpors?
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.Format = DefaultDepthReadformat;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		CurrentDevice->GetDevice()->CreateShaderResourceView(DepthStencil->GetResource(), &shadowSrvDesc, SrvHeap->GetCPUAddress(0));


		NullHeap->SetName(L"Shadow SRV heap");

		CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &shadowSrvDesc, NullHeap->GetCPUAddress(0));

	}
}

void D3D12FrameBuffer::CreateGBuffer()
{
	RTVformat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	CreateDepth();
	for (int i = 0; i < RenderTargetCount; i++)
	{
		CreateColour(i);
	}
}

bool D3D12FrameBuffer::CheckDevice(int index)
{
	if (CurrentDevice != nullptr)
	{
		if (OtherDevice != nullptr)
		{
			return (OtherDevice->GetDeviceIndex() == index) || (CurrentDevice->GetDeviceIndex() == index);
		}
		return (CurrentDevice->GetDeviceIndex() == index);
	}
	return false;
}

void D3D12FrameBuffer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	//todo: ensure not in use!
	if (m_ftype == FrameBufferType::ColourDepth)
	{
		DepthStencil->Release();
		delete DepthStencil;
		CreateDepth();
	}

	if (m_ftype == FrameBufferType::ColourDepth || m_ftype == FrameBufferType::Colour)
	{
		RenderTarget[0]->GetResource()->Release();
		CreateColour();
	}

	if (m_ftype == FrameBufferType::GBuffer)
	{
		for (int i = 0; i < RenderTargetCount; i++)
		{
			RenderTarget[i]->Release();
		}
		DepthStencil->Release();
		CreateGBuffer();
	}
}

static inline UINT Align(UINT size, UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
{
	return (size + alignment - 1) & ~(alignment - 1);
}

void D3D12FrameBuffer::SetupCopyToDevice(DeviceContext * device)
{
	OtherDevice = device;
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();
	renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(RTVformat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
#if 1

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;

	Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &layout, nullptr, nullptr, nullptr);
	UINT64 textureSize = Align(layout.Footprint.RowPitch * layout.Footprint.Height);

	// Create a buffer with the same layout as the render target texture.
	D3D12_RESOURCE_DESC crossAdapterDesc = CD3DX12_RESOURCE_DESC::Buffer(textureSize, D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER);

	CD3DX12_HEAP_DESC heapDesc(
		textureSize,
		D3D12_HEAP_TYPE_DEFAULT,
		0,
		D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);
	Host->CreateHeap(&heapDesc, IID_PPV_ARGS(&CrossHeap));



	ThrowIfFailed(Host->CreateSharedHandle(
		CrossHeap,
		nullptr,
		GENERIC_ALL,
		nullptr,
		&heapHandle));

	HRESULT openSharedHandleResult = Target->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&TWO_CrossHeap));

	// We can close the handle after opening the cross-adapter shared resource.
	CloseHandle(heapHandle);
	//target
	ThrowIfFailed(Host->CreatePlacedResource(
		CrossHeap,
		0,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&PrimaryRes)));
	//source
	ThrowIfFailed(Target->CreatePlacedResource(
		TWO_CrossHeap,
		0,
		&crossAdapterDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		nullptr,
		IID_PPV_ARGS(&Stagedres)));
#endif
	////final out
	//ThrowIfFailed(Primary->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE,
	//	&renderTargetDesc,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//	nullptr,
	//	IID_PPV_ARGS(&FinalOut)));
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = RTVformat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;
	ThrowIfFailed(Target->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(RTVformat, m_width, m_height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&FinalOut)
	));

	SharedSRVHeap = new DescriptorHeap(OtherDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//	SharedSRVHeap->
	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = RTVformat;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SrvDesc.Texture2D.MipLevels = 1;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	OtherDevice->GetDevice()->CreateShaderResourceView(FinalOut, &SrvDesc, SharedSRVHeap->GetCPUAddress(0));
	SharedTarget = new GPUResource(FinalOut, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void D3D12FrameBuffer::CopyToDevice(ID3D12GraphicsCommandList* list)
{
	//return;
	// Copy the intermediate render target into the shared buffer using the
	// memory layout prescribed by the render target.
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT renderTargetLayout;

	Host->GetCopyableFootprints(&renderTargetDesc, 0, 1, 0, &renderTargetLayout, nullptr, nullptr, nullptr);

	CD3DX12_TEXTURE_COPY_LOCATION dest(PrimaryRes, renderTargetLayout);
	RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_COPY_SOURCE);
	CD3DX12_TEXTURE_COPY_LOCATION src(RenderTarget[0]->GetResource(), 0);
	CD3DX12_BOX box(0, 0, m_width, m_height);

	list->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);
	RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void D3D12FrameBuffer::MakeReadyOnTarget(ID3D12GraphicsCommandList* list)
{
	//return;
	ID3D12Device* Host = CurrentDevice->GetDevice();
	ID3D12Device* Target = OtherDevice->GetDevice();
	// Copy the buffer in the shared heap into a texture that the secondary
	// adapter can sample from.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		FinalOut,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COPY_DEST);
	list->ResourceBarrier(1, &barrier);

	//list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(FinalOut, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE,0));

	//list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(	PrimaryRes, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE));
	// Copy the shared buffer contents into the texture using the memory
	// layout prescribed by the texture.
	D3D12_RESOURCE_DESC secondaryAdapterTexture = FinalOut->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureLayout;

	Host->GetCopyableFootprints(&secondaryAdapterTexture, 0, 1, 0, &textureLayout, nullptr, nullptr, nullptr);

	CD3DX12_TEXTURE_COPY_LOCATION dest(FinalOut, 0);
	CD3DX12_TEXTURE_COPY_LOCATION src(Stagedres, textureLayout);
	CD3DX12_BOX box(0, 0, m_width, m_height);

	list->CopyTextureRegion(&dest, 0, 0, 0, &src, &box);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	list->ResourceBarrier(1, &barrier);
}


void D3D12FrameBuffer::BindDepthWithColourPassthrough(ID3D12GraphicsCommandList * list, D3D12FrameBuffer * Passtrhough)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
	if (Passtrhough->RenderTarget[0])
	{
		Passtrhough->RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	list->OMSetRenderTargets(Passtrhough->RenderTargetCount, &Passtrhough->RTVHeap->GetCPUAddress(0), true, &DSVHeap->GetCPUAddress(0));

}

D3D12FrameBuffer::~D3D12FrameBuffer()
{
	if (RequiresDepth())
	{
		DepthStencil->Release();
		delete NullHeap;
		delete DSVHeap;
	}
	if (RenderTargetCount > 0)
	{
		delete RTVHeap;
	}
	for (int i = 0; i < RenderTargetCount; i++)
	{
		RenderTarget[i]->Release();
	}
	delete SrvHeap;
}
void D3D12FrameBuffer::ReadyResourcesForRead(CommandListDef * list, int Resourceindex)
{
	if (RenderTarget[Resourceindex] != nullptr)
	{
		RenderTarget[Resourceindex]->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	if (DepthStencil != nullptr && m_ftype == Depth)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void D3D12FrameBuffer::BindBufferToTexture(CommandListDef * list, int slot, int Resourceindex, DeviceContext* target)
{
	/*if (m_srvHeap == nullptr)
	{
		return;
	}*/

	lastboundslot = slot;

	
	if (target != nullptr && OtherDevice != nullptr)
	{
		if (target == OtherDevice)
		{
			SharedSRVHeap->BindHeap(list);
			list->SetGraphicsRootDescriptorTable(slot, SharedSRVHeap->GetGpuAddress(Resourceindex));
			return;
		}
	}
	SrvHeap->BindHeap(list);
	ReadyResourcesForRead(list, Resourceindex);
	list->SetGraphicsRootDescriptorTable(slot, SrvHeap->GetGpuAddress(Resourceindex));

}

void D3D12FrameBuffer::BindBufferAsRenderTarget(CommandListDef * list)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);

	if (m_ftype == FrameBufferType::CubeDepth)
	{
		RenderTarget[0]->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		list->OMSetRenderTargets(0, nullptr, false, &RTVHeap->GetCPUAddress(0));
	}
	else
	{
		for (int i = 0; i < RenderTargetCount; i++)
		{
			if (RenderTarget[i] != nullptr)
			{
				RenderTarget[i]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
		}
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
	if (m_ftype != FrameBufferType::CubeDepth)
	{
		if (RequiresDepth())
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE();
			if (RTVHeap)
			{
				//validate this is okay todo?
				rtvHandle = RTVHeap->GetCPUAddress(0);
			}
			list->OMSetRenderTargets(RenderTargetCount, &rtvHandle, (m_ftype == FrameBufferType::GBuffer), &DSVHeap->GetCPUAddress(0));
		}
		else
		{
			list->OMSetRenderTargets(RenderTargetCount, &RTVHeap->GetCPUAddress(0), (m_ftype == FrameBufferType::GBuffer), nullptr);
		}
	}

}

void D3D12FrameBuffer::UnBind(CommandListDef * list)
{
	if (NullHeap == nullptr)
	{
		return;
	}
	NullHeap->BindHeap(list);
	NullHeap->SetName(L"null  SRV");
	list->SetGraphicsRootDescriptorTable(lastboundslot, NullHeap->GetGpuAddress(0));
}

void D3D12FrameBuffer::ClearBuffer(CommandListDef * list)
{
	if (RequiresDepth())
	{
		list->ClearDepthStencilView(DSVHeap->GetCPUAddress(0), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	if (m_ftype == ColourDepth || m_ftype == GBuffer || m_ftype == Colour)
	{

		for (int i = 0; i < RenderTargetCount; i++)
		{
			list->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), &BufferClearColour[0], 0, nullptr);
		}

	}
	if (m_ftype == CubeDepth)
	{
		list->ClearDepthStencilView(RTVHeap->GetCPUAddress(0), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//	list->ClearRenderTargetView(RTVHeap->GetCPUAddress(0), CubeDepthclearColor, 0, nullptr);
			/*for (int i = 0; i < CUBE_SIDES; i++)
			{

			}*/
	}
}


D3D12Shader::PipeRenderTargetDesc D3D12FrameBuffer::GetPiplineRenderDesc()
{
	D3D12Shader::PipeRenderTargetDesc output = {};
	switch (m_ftype)
	{
	case ColourDepth:
		output.RTVFormats[0] = RTVformat;
		output.NumRenderTargets = 0;
		output.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	case Depth:
		output.RTVFormats[0] = RTVformat;
		output.NumRenderTargets = RenderTargetCount;
		output.DSVFormat = Depthformat;
		break;
	case Colour:
		output.RTVFormats[0] = RTVformat;
		output.NumRenderTargets = RenderTargetCount;
		output.DSVFormat = Depthformat;
		break;
	case CubeDepth:
		output.NumRenderTargets = 0;
		output.DSVFormat = Depthformat;
		break;
	case GBuffer:
		RTVformat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		output.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		output.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		output.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
		output.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
		output.DSVFormat = Depthformat;
		output.NumRenderTargets = RenderTargetCount;
		break;
	}
	return output;
}

