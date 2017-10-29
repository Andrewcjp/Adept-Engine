#include "D3D11Cube.h"
#if BUILD_D3D11
#include "D3D11Texture.h"

D3D11Cube::D3D11Cube(D3D11ShaderProgram* shader)
{
	m_shader = shader;

	InitUnitCube ( shader );
}

D3D11Cube::~D3D11Cube()
{
	if ( m_vertexLayout ) m_vertexLayout->Release();

	if ( m_vertexBuffer ) m_vertexBuffer->Release();
}
		
void D3D11Cube::InitUnitCube(D3D11ShaderProgram* shader)
{
	//todo: Make a plane!
	const float scale = 0.9f;
	const float ZScale = 0.2f;
	SimpleVertex corners[] = 
	{
		{-scale, -scale, ZScale, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},// 0
		{scale, -scale, ZScale, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f}, // 1
		{scale, scale, ZScale, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f}, // 2
		{-scale, scale, ZScale, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // 3

		{scale, -scale, ZScale, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // 4
		{scale, -scale, -ZScale, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f}, // 5
		{scale, scale, -ZScale, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // 6
		{scale, scale, ZScale, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // 7

		{scale, -scale, -ZScale, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f}, // 8
		{-scale, -scale, -ZScale, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f}, //9
		{-scale, scale, -ZScale, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // 10
		{scale, scale, -ZScale, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 11
		
		{-scale, -scale, -ZScale, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}, //12
		{-scale, -scale, ZScale, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},// 13
		{-scale, scale, ZScale, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}, // 14
		{-scale, scale, -ZScale, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 15

		{-scale, scale, ZScale, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f}, // 16
		{scale, scale, ZScale, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f}, // 17
		{scale, scale, -ZScale, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // 18
		{-scale, scale, -ZScale, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 19
	
		{-scale, -scale, -ZScale, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}, //20
		{scale, -scale, -ZScale, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f}, // 21
		{scale, -scale, ZScale, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}, // 22
		{-scale, -scale, ZScale, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}// 23
	};
	
	//We will use triangles instead of triangle strips
	unsigned int indices[] = 
	{
		0, 3, 1, 
		1, 3, 2,

		4, 7, 5, 
		7, 6, 5,

		8, 11, 9, 
		9, 11, 10,

		12, 15, 14, 
		12, 14, 13,

		16, 19, 18, 
		16, 18, 17,

		20, 23, 22, 
		20, 22, 21
	};

	D3D11_INPUT_ELEMENT_DESC vertlayout [] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOUR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	
	ID3D11Device* pDevice = shader->GetDevice();
	ID3DBlob* pVSBlob = shader->GetVSBlob();

	pDevice->CreateInputLayout( vertlayout, ARRAYSIZE(vertlayout), pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_vertexLayout );

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex)*24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory( &initData, sizeof(initData) );
	initData.pSysMem = corners;

	pDevice->CreateBuffer( &bd, &initData, &m_vertexBuffer );
	
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned int)*36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = indices;

	pDevice->CreateBuffer( &bd, &initData, &m_indexBuffer );
}
	
void D3D11Cube::Render()
{
	ID3D11DeviceContext* pContext = m_shader->GetContext();
	//ID3D11ShaderResourceView* ptexRv = dynamic_cast<D3D11Texture*>(m_tex)->m_textureRV;

	unsigned int stride = sizeof(SimpleVertex);
	unsigned int offset = 0;

	pContext->IASetInputLayout( m_vertexLayout );
	pContext->IASetVertexBuffers( 0, 1, &m_vertexBuffer, &stride, &offset );
	pContext->IASetIndexBuffer( m_indexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	
	//pContext->PSSetShaderResources( 0, 1, &ptexRv );

	pContext->DrawIndexed( 36, 0, 0 );
}
#endif