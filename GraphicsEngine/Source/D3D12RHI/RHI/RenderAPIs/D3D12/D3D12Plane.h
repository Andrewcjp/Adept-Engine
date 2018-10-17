#pragma once

#include "Rendering/Core/Mesh.h"
#include <DirectXMath.h>
#include <d3d12.h>

class D3D12Plane : public Mesh
{
public:
	D3D12Plane(float size);
	~D3D12Plane();

	// Inherited via Mesh
	//virtual void Render(ID3D12GraphicsCommandList* list = nullptr) override;
	//void Render(RHICommandList* list)override;
	void CreateVertexBuffer();
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 color;
	};
	int m_numtriangles = 0;
	ID3D12Resource* m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};

