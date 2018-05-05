#include "stdafx.h"
#include "D3D12Framebuffer.h"
#include "D3D12RHI.h"
#include "GPUResource.h"
#define CUBE_SIDES 6
#include "../RHI/DeviceContext.h"
void D3D12FrameBuffer::CreateCubeDepth()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = CUBE_SIDES+1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

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
		IID_PPV_ARGS(&m_RenderTarget)
	));

	RenderTarget = new GPUResource(m_RenderTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = RTVformat;
	renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.MipSlice = 0;
	renderTargetViewDesc.Texture2DArray.ArraySize = CUBE_SIDES;
	renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;

	CurrentDevice->GetDevice()->CreateRenderTargetView(m_RenderTarget, &renderTargetViewDesc, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	renderTargetViewDesc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < CUBE_SIDES; i++)
	{	
		renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;
		CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), i+1, descriptorSize);
		CurrentDevice->GetDevice()->CreateRenderTargetView(m_RenderTarget, &renderTargetViewDesc, currentCPUHandle);
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
	CurrentDevice->GetDevice()->CreateShaderResourceView(m_RenderTarget, &shadowSrvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	m_srvHeap->SetName(L"Shadow 3d  SRV heap");

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_nullHeap)));
	NullHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_nullHeap->GetGPUDescriptorHandleForHeapStart());
	CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &shadowSrvDesc, m_nullHeap->GetCPUDescriptorHandleForHeapStart());
}
void D3D12FrameBuffer::CreateColour()
{
	RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = RTVformat;
	renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = renderTargetViewDesc.Format;
	depthOptimizedClearValue.Color[0] = clearColor[0];
	depthOptimizedClearValue.Color[1] = clearColor[1];
	depthOptimizedClearValue.Color[2] = clearColor[2];
	depthOptimizedClearValue.Color[3] = clearColor[3];

	ThrowIfFailed(CurrentDevice->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(renderTargetViewDesc.Format, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&m_RenderTarget)
	));

	RenderTarget = new GPUResource(m_RenderTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CurrentDevice->GetDevice()->CreateRenderTargetView(m_RenderTarget, &renderTargetViewDesc, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	CreateSRV();
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.Format = renderTargetViewDesc.Format;
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shadowSrvDesc.Texture2D.MipLevels = 1;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	CurrentDevice->GetDevice()->CreateShaderResourceView(m_RenderTarget, &shadowSrvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

}
void D3D12FrameBuffer::CreateSRV()
{
	if (m_srvHeap == nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
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
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	{
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
	}
	CreateSRV();
	if (m_ftype == Depth)
	{

		//todo look at usage of heaps?	

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
D3D12FrameBuffer::~D3D12FrameBuffer()
{
	if (m_ftype == FrameBufferType::Depth)
	{
		m_depthStencil->Release();
		m_nullHeap->Release();
		m_dsvHeap->Release();
		m_srvHeap->Release();
	}
}

void D3D12FrameBuffer::BindToTextureUnit(int unit)
{}

void D3D12FrameBuffer::BindBufferToTexture(CommandListDef * list, int slot)
{
	if (m_srvHeap == nullptr)
	{
		return;
	}
	lastboundslot = slot;

	if (RenderTarget != nullptr)
	{
		RenderTarget->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	list->SetGraphicsRootDescriptorTable(slot, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void D3D12FrameBuffer::BindBufferAsRenderTarget(CommandListDef * list)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);

	if (RenderTarget != nullptr)
	{
		RenderTarget->SetResourceState(list,D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	if (DepthStencil != nullptr)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	if (m_ftype == Depth)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		list->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	}
	else if (m_ftype == ColourDepth)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		list->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	}
	if (m_ftype == CubeDepth)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		list->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
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
	if (m_ftype == Depth || m_ftype == ColourDepth)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	if (m_ftype == ColourDepth)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		list->ClearRenderTargetView(rtHandle, clearColor, 0, nullptr);
	}
	if (m_ftype == CubeDepth)
	{
		for (int i = 0; i < CUBE_SIDES; i++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),i, descriptorSize);
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
		break;
	}
	return output;
}

