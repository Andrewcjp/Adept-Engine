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
	RTVHeap = new DescriptorHeap(CurrentDevice, CUBE_SIDES + 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	ID3D12Resource * NewRenderTarget;
	RTVformat = DXGI_FORMAT_R32_FLOAT;
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_R32_FLOAT;
	depthOptimizedClearValue.Color[0] = CubeDepthclearColor[0];
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

	CurrentDevice->GetDevice()->CreateRenderTargetView(NewRenderTarget, &renderTargetViewDesc, RTVHeap->GetCPUAddress(0));
	renderTargetViewDesc.Texture2DArray.ArraySize = 1;
	for (int i = 0; i < CUBE_SIDES; i++)
	{
		renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;
		CurrentDevice->GetDevice()->CreateRenderTargetView(NewRenderTarget, &renderTargetViewDesc, RTVHeap->GetCPUAddress(i+1));
		//might be a read over end issue here
	}

	SrvHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	SrvHeap->SetName(L"DSV Heap");
	
	D3D12_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
	shadowSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	shadowSrvDesc.Texture2D.MipLevels = 1;
	shadowSrvDesc.Texture2DArray.ArraySize = CUBE_SIDES;
	shadowSrvDesc.Texture2DArray.MipLevels = 1;
	shadowSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	CurrentDevice->GetDevice()->CreateShaderResourceView(NewRenderTarget, &shadowSrvDesc, SrvHeap->GetCPUAddress(0));
	SrvHeap->SetName(L"Shadow 3d  SRV heap");
	
	NullHeap = new DescriptorHeap(CurrentDevice, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CurrentDevice->GetDevice()->CreateShaderResourceView(nullptr, &shadowSrvDesc, NullHeap->GetCPUAddress(0));
}

void D3D12FrameBuffer::CreateColour(int Index )
{
	if (Index ==2)
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
		&CD3DX12_RESOURCE_DESC::Tex2D(renderTargetViewDesc.Format, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET),
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
	Depthformat = DXGI_FORMAT_D32_FLOAT;
	if (DSVHeap == nullptr)
	{
		DSVHeap = new DescriptorHeap(CurrentDevice,  1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
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
	CurrentDevice->GetDevice()->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, DSVHeap->GetCPUAddress(0));
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
		CurrentDevice->GetDevice()->CreateShaderResourceView(m_depthStencil, &shadowSrvDesc, SrvHeap->GetCPUAddress(0));
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
		delete NullHeap;
		delete DSVHeap;
	}
	if (m_ftype == FrameBufferType::Colour)
	{		
		delete RTVHeap;
		
	}
	delete SrvHeap;
}

void D3D12FrameBuffer::BindBufferToTexture(CommandListDef * list, int slot,int Resourceindex)
{
	/*if (m_srvHeap == nullptr)
	{
		return;
	}*/
	lastboundslot = slot;


	if (RenderTarget[Resourceindex] != nullptr)
	{
			RenderTarget[Resourceindex]->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	
	if (DepthStencil != nullptr && m_ftype == Depth)
	{
		DepthStencil->SetResourceState(list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	SrvHeap->BindHeap(list);
	list->SetGraphicsRootDescriptorTable(slot, SrvHeap->GetGpuAddress(Resourceindex));

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
		if (RTVHeap)
		{
			//validate this is okay todo?
			rtvHandle = RTVHeap->GetCPUAddress(0);
		}		
		list->OMSetRenderTargets(RenderTargetCount, &rtvHandle, true, &DSVHeap->GetCPUAddress(0));
	}
	else
	{
		list->OMSetRenderTargets(RenderTargetCount, &RTVHeap->GetCPUAddress(0), true, nullptr);
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
		for (int i = 0; i < CUBE_SIDES; i++)
		{
			list->ClearRenderTargetView(RTVHeap->GetCPUAddress(i), CubeDepthclearColor, 0, nullptr);
		}
	}
}


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

