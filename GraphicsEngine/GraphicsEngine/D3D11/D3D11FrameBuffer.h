#pragma once
#include "EngineGlobals.h"
#if BUILD_D3D11
#include "Rendering/Core/FrameBuffer.h"
#include <d3d11_1.h>
class D3D11FrameBuffer :
	public FrameBuffer
{
public:
	D3D11FrameBuffer(int width, int height, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth) :FrameBuffer(width, height, ratio, type)
	{
		CreateBuffer();
	}
	virtual ~D3D11FrameBuffer();
	void CreateColour();
	void CreateBuffer();
	// Inherited via FrameBuffer
	virtual void BindToTextureUnit(int unit = 0) override;
	virtual void BindBufferAsRenderTarget(CommandListDef * list = nullptr) override;
	virtual void UnBind() override;
	void ClearBuffer(CommandListDef * list = nullptr)override;
	void CreateDepth();
	void CreateRenderDepth();
private:

	ID3D11Texture2D *Texture;
	ID3D11RenderTargetView * View;
	ID3D11ShaderResourceView * shaderview;
	ID3D11Texture2D				*depthStencil;
	ID3D11DepthStencilView * DepthView;
};
#endif
