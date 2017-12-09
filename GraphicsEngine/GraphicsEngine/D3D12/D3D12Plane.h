#pragma once
#include "../Rendering/Core/Renderable.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include "../EngineGlobals.h"
class D3D12Plane : public Renderable
{
public:
	D3D12Plane(float size);
	~D3D12Plane();

	// Inherited via Renderable
	//virtual void Render(CommandListDef* list = nullptr) override;
	void Render(CommandListDef* list)override;
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

