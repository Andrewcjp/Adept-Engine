#include "D3D11FrameBuffer.h"
#if BUILD_D3D11
#include "RHI/RHI.h"
#include "glm\glm.hpp"
D3D11FrameBuffer::~D3D11FrameBuffer()
{
	if (View != nullptr)
	{
		View->Release();
		depthStencil->Release();
		DepthView->Release();
	}
}

void D3D11FrameBuffer::CreateBuffer()
{
	if (m_ftype == ColourDepth)
	{
		CreateColour();
		CreateRenderDepth();
	}
	else if (m_ftype == Depth)
	{
		CreateDepth();
	}
}

void D3D11FrameBuffer::BindToTextureUnit(int unit)
{
	RHI::GetD3DContext()->PSSetSamplers(0, 1, &RHI::instance->m_texSamplerFBState);
	RHI::GetD3DContext()->PSSetShaderResources(0, 1, &shaderview);
}

void D3D11FrameBuffer::BindBufferAsRenderTarget(CommandListDef * list)
{
	//improtant
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(m_width);
	viewport.Height = static_cast<FLOAT>(m_height);
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	float aspectratio = (float)m_width / (float)m_height;
	RHI::GetD3DContext()->RSSetViewports(1, &viewport);
	RHI::GetD3DContext()->OMSetRenderTargets(1, &View, DepthView);

}

void D3D11FrameBuffer::UnBind()
{
	ID3D11ShaderResourceView * tab[1];
	tab[0] = NULL;
	RHI::GetD3DContext()->PSSetShaderResources(0, 1, tab);
}

void D3D11FrameBuffer::ClearBuffer(CommandListDef * list)
{
	float clearcolour[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
	if (View != nullptr)
	{
		RHI::GetD3DContext()->ClearRenderTargetView(View, clearcolour);
	}
	if (DepthView != nullptr)
	{
		//ensure 1.0!
		RHI::GetD3DContext()->ClearDepthStencilView(DepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

}
void D3D11FrameBuffer::CreateDepth()
{
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_width;
	descDepth.Height = m_height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	HRESULT result = RHI::GetD3DDevice()->CreateTexture2D(&descDepth, NULL, &depthStencil);
	D3DEnsure(result);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	result = RHI::GetD3DDevice()->CreateDepthStencilView(depthStencil, &descDSV, &DepthView);
	D3DEnsure(result);
	if (m_ftype == Depth)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		result = RHI::GetD3DDevice()->CreateShaderResourceView(depthStencil, &shaderResourceViewDesc, &shaderview);
		D3DEnsure(result);
	}
	//D3D11_VIEWPORT viewport;
	//D3D11 WARNING: ID3D11DeviceContext::Draw: The Pixel Shader expects a Render Target View bound to slot 0, but none is bound. This is OK, as writes of an unbound Render Target View are discarded. It is also possible the developer knows the data will not be used anyway. This is only a problem if the developer actually intended to bind a Render Target View here. [ EXECUTION WARNING #3146081: DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET]
	//viewport.TopLeftX = 0;
	//viewport.TopLeftY = 0;
	//viewport.Width = static_cast<FLOAT>(m_width);
	//viewport.Height = static_cast<FLOAT>(m_height);
	//viewport.MaxDepth = 1.0f;
	//viewport.MinDepth = 0.0f;

	//float aspectratio = (float)m_width / (float)m_height;
	//RHI::GetD3DContext()->SO
}
void D3D11FrameBuffer::CreateRenderDepth()
{
	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = m_width;
	descDepth.Height = m_height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	HRESULT  result = RHI::GetD3DDevice()->CreateTexture2D(&descDepth, nullptr, &depthStencil);
	D3DEnsure(result);
	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	result = RHI::GetD3DDevice()->CreateDepthStencilView(depthStencil, &descDSV, &DepthView);
	D3DEnsure(result);
}
void D3D11FrameBuffer::CreateColour()
{
	//create a render target withj 
	D3D11_TEXTURE2D_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ArraySize = 1;
	bufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	bufferDesc.Height = m_height;
	bufferDesc.MipLevels = 1;
	bufferDesc.MiscFlags = 0;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.Width = m_width;
	HRESULT result = RHI::GetD3DDevice()->CreateTexture2D(&bufferDesc, NULL, &Texture);
	D3DEnsure(result);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	RHI::GetD3DDevice()->CreateRenderTargetView(Texture, &renderTargetViewDesc, &View);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = bufferDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	result = RHI::GetD3DDevice()->CreateShaderResourceView(Texture, &shaderResourceViewDesc, &shaderview);
	D3DEnsure(result);
	//Texture->Release();

	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(m_width);
	viewport.Height = static_cast<FLOAT>(m_height);
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	float aspectratio = (float)m_width / (float)m_height;
	RHI::GetD3DContext()->RSSetViewports(1, &viewport);
}
#endif