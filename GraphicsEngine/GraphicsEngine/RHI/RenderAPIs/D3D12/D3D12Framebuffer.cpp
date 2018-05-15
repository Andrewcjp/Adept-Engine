#include "stdafx.h"
#include "D3D12Framebuffer.h"
#include "D3D12RHI.h"
#include "GPUResource.h"
#define CUBE_SIDES 6
#include "../RHI/DeviceContext.h"
#include <algorithm>
void D3D12FrameBuffer::CreateCubeDepth()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = CUBE_SIDES + 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	ID3D12Resource * NewRenderTarget;
	RTVformat = DXGI_FORMAT_R32_FLOAT;
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_R32_FLOAT;
	depthOptimizedClearValue.Color[0] = CubeDepthclearColor[0];
	descriptorSize = CurrentDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_FLOAT, m_width, m_height, 6, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&NewRenderTarget)
	));

	RenderTarget[0] = new GPUResource(NewRenderTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = RTVformat;
	renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.ArraySize = CUBE_SIDES;
	renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;

	CurrentDevice->GetDevice()->CreateRenderTargetView(NewRenderTarget, &renderTargetViewDesc, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	renderTargetViewDesc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < CUBE_SIDES; i++)
	{
		renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;
		CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i + 1, descriptorSize);
		CurrentDevice->GetDevice()->CreateRenderTargetView(NewRenderTarget, &renderTargetViewDesc, currentCPUHandle);
		//might be a read over end issue here
	}


	//todo look at usage of heaps?
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));


	m_srvHeap->SetName(L"Depth SRV");
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	shadowSrvDesc.Texture2D.MipLevels = 1;
	shadowSrvDesc.Texture2DArray.ArraySize = CUBE_SIDES;
	shadowSrvDesc.Texture2DArray.MipLevels = 1;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	CurrentDevice->GetDevice()->CreateShaderResourceView(NewRenderTarget, &shadowSrvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	m_srvHeap->SetName(L"Shadow 3d  SRV heap");

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_nullHeap)));
	NullHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_nullHeap->GetGPUDescriptorHandleForHeapStart());
	CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &shadowSrvDesc, m_nullHeap->GetCPUDescriptorHandleForHeapStart());
}
void D3D12FrameBuffer::CreateColour(int Index )
{
	if (Index ==2)
	{
		RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	ID3D12Resource * NewRenderTarget;
	if (m_rtvHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = RenderTargetCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
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
		&CD3DX12_RESOURCE_DESC::Tex2D(renderTargetViewDesc.Format, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&NewRenderTarget)
	));

	RenderTarget[Index] = new GPUResource(NewRenderTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(Index, CurrentDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	CurrentDevice->GetDevice()->CreateRenderTargetView(NewRenderTarget, &renderTargetViewDesc, rtvHandle);
	CreateSRV();
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.Format = renderTargetViewDesc.Format; 
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shadowSrvDesc.Texture2D.MipLevels = 1;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	srvHandle.Offset(Index, CurrentDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	CurrentDevice->GetDevice()->CreateShaderResourceView(NewRenderTarget, &shadowSrvDesc, srvHandle);

}
void D3D12FrameBuffer::CreateSRV()
{
	if (m_srvHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = std::max(RenderTargetCount,1);
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
		m_srvHeap->SetName(L"Depth SRV");
		if (m_ftype == Depth)
		{
			ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_nullHeap)));
			NullHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_nullHeap->GetGPUDescriptorHandleForHeapStart());
		}
	}
}
void D3D12FrameBuffer::CreateDepth()
{
	Depthformat = DXGI_FORMAT_D32_FLOAT;
	if (m_dsvHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = Depthformat;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = Depthformat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(Depthformat, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_depthStencil)
	));
	DepthStencil = new GPUResource(m_depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	//NAME_D3D12_OBJECT(m_depthStencil);
	DepthHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	CurrentDevice->GetDevice()->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, DepthHandle);
	m_depthStencil->SetName(L"FrameBuffer Stencil");

	CreateSRV();
	if (m_ftype == Depth)
	{

		//todo look at usage of heaps?	
		//todo: resize might leak descritpors?
		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		CurrentDevice->GetDevice()->CreateShaderResourceView(m_depthStencil, &shadowSrvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
		m_srvHeap->SetName(L"Shadow SRV heap");

		CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &shadowSrvDesc, m_nullHeap->GetCPUDescriptorHandleForHeapStart());

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
		m_depthStencil->Release();
		delete DepthStencil;
		CreateDepth();


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

void D3D12FrameBuffer::SetupCopyToDevice(DeviceContext * device)
{

}

void D3D12FrameBuffer::CopyToDevice(DeviceContext * device)
{

}

D3D12FrameBuffer::~D3D12FrameBuffer()
{
	if (RequiresDepth())
	{
		m_depthStencil->Release();
	}
	if (m_ftype == FrameBufferType::Depth)
	{

		m_nullHeap->Release();
		m_dsvHeap->Release();
		m_srvHeap->Release();
	}
}



void D3D12FrameBuffer::BindBufferToTexture(CommandListDef * list, int slot,int Resourceindex)
{
	if (m_srvHeap == nullptr)
	{
		return;
	}
	lastboundslot = slot;


	if (RenderTarget[Resourceindex] != nullptr)
	{
			RenderTarget[Resourceindex]->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	
	if (DepthStencil != nullptr && m_ftype == Depth)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	srvHandle.Offset(Resourceindex, CurrentDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	list->SetGraphicsRootDescriptorTable(slot, srvHandle);
}

void D3D12FrameBuffer::BindBufferAsRenderTarget(CommandListDef * list)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);

	
	for (int i = 0; i < RenderTargetCount; i++)
	{
		if (RenderTarget[i] != nullptr)
		{
			RenderTarget[i]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}		
	}

	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	if (RequiresDepth())
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE();
		if (m_rtvHeap)
		{
			//validate this is okay todo?
			rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		}		
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		list->OMSetRenderTargets(RenderTargetCount, &rtvHandle, true, &dsvHandle);
	}
	else
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		list->OMSetRenderTargets(RenderTargetCount, &rtvHandle, true, nullptr);
	}


}

void D3D12FrameBuffer::UnBind(CommandListDef * list)
{
	if (m_nullHeap == nullptr)
	{
		return;
	}
	ID3D12DescriptorHeap* ppHeaps[] = { m_nullHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_nullHeap->SetName(L"null  SRV");
	list->SetGraphicsRootDescriptorTable(lastboundslot, m_nullHeap->GetGPUDescriptorHandleForHeapStart());
}

void D3D12FrameBuffer::ClearBuffer(CommandListDef * list)
{
	if (RequiresDepth())
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	if (m_ftype == ColourDepth || m_ftype == GBuffer)
	{
		
		for (int i = 0; i < RenderTargetCount; i++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
			rtHandle.Offset(i, CurrentDevice->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
			list->ClearRenderTargetView(rtHandle, &BufferClearColour[0], 0, nullptr);
		}
		
	}
	if (m_ftype == CubeDepth)
	{
		for (int i = 0; i < CUBE_SIDES; i++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, descriptorSize);
			list->ClearRenderTargetView(rtHandle, CubeDepthclearColor, 0, nullptr);
		}
	}
}
//todo: RHI this!
//But check Implemntion once Defferred done!
D3D12Shader::PipeRenderTargetDesc D3D12FrameBuffer::GetPiplineRenderDesc()
{
	D3D12Shader::PipeRenderTargetDesc output = {};
	switch (m_ftype)
	{
	case ColourDepth:
		output.RTVFormats[0] = RTVformat;
	case Depth:
		output.DSVFormat = Depthformat;
		break;
	case CubeDepth:
		output.DSVFormat = DXGI_FORMAT_UNKNOWN;
		output.NumRenderTargets = CUBE_SIDES;
		for (int i = 0; i < CUBE_SIDES; i++)
		{
			output.RTVFormats[i] = RTVformat;
		}
		break;
	case GBuffer:
		output.RTVFormats[0] = RTVformat;
		output.RTVFormats[1] = RTVformat;
		output.RTVFormats[2] = RTVformat;
		output.RTVFormats[3] = RTVformat;
		output.DSVFormat = Depthformat;
		output.NumRenderTargets = RenderTargetCount;
		break;
	}
	return output;
}

