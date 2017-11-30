#include "stdafx.h"
#include "D3D12Framebuffer.h"
#include "D3D12RHI.h"



void D3D12FrameBuffer::CreateBuffer()
{
	if (m_ftype == ColourDepth)
	{
		//CreateColour();
	//	CreateRenderDepth();
	}
	else if (m_ftype == Depth)
	{
		CreateDepth();
	}
}
void D3D12FrameBuffer::CreateDepth()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
	//m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));

		//NAME_D3D12_OBJECT(m_depthStencil);
		DepthHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12RHI::GetDevice()->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, DepthHandle);
	}
	if (m_ftype == Depth)
	{

		//todo look at usage of heaps?
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 2;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(D3D12RHI::Instance->m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));



		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
		shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSrvDesc.Texture2D.MipLevels = 1;
		shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		D3D12RHI::GetDevice()->CreateShaderResourceView(m_depthStencil, &shadowSrvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	}
}
D3D12FrameBuffer::~D3D12FrameBuffer()
{
}

void D3D12FrameBuffer::BindToTextureUnit(int unit)
{
}

void D3D12FrameBuffer::BindBufferToTexture(CommandListDef * list)
{
	//list->SetGraphicsRootDescriptorTable(2, m_depthStencil->GetGPUVirtualAddress());
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	list->SetGraphicsRootDescriptorTable(4, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	//list->SetGraphicsRootShaderResourceView(3,m_depthStencil->GetGPUVirtualAddress());
}

void D3D12FrameBuffer::BindBufferAsRenderTarget(CommandListDef * list)
{
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
	list->OMSetRenderTargets(0, nullptr, FALSE, &dsvHandle);
}

void D3D12FrameBuffer::UnBind(CommandListDef * list)
{
	const UINT cbvSrvDescriptorSize = D3D12RHI::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_GPU_DESCRIPTOR_HANDLE null(m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	null.Offset(1, cbvSrvDescriptorSize);
	//list->SetGraphicsRootDescriptorTable(0, null);
}

void D3D12FrameBuffer::ClearBuffer(CommandListDef * list)
{
	if (m_ftype == Depth)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

