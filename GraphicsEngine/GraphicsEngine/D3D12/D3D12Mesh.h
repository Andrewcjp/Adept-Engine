#pragma once
#include "../Rendering/Core/Renderable.h"
#include <DirectXMath.h>
#include <d3d12.h>
class D3D12Mesh :public Renderable
{
public:
	D3D12Mesh();
	virtual ~D3D12Mesh();

	// Inherited via Renderable
	virtual void Render() override;
	void Render(ID3D12GraphicsCommandList* list);
	void CreateVertexBuffer();
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 color;
	};
	ID3D12Resource* m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};

