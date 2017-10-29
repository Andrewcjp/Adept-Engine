#include "EngineGlobals.h"
#if BUILD_D3D11
#include <d3d11_1.h>
#include "D3D11Mesh.h"
#include "D3D11Texture.h"
#include "Rendering/Core/Triangle.h"
#include "Core/Assets/OBJFileReader.h"
#include "glm\glm.hpp"
#include "RHI/RHI.h"
D3D11Mesh::D3D11Mesh()
{

}
//D3D11Mesh::D3D11Mesh(LPCWSTR filename, D3D11ShaderProgram* shader)
//{
//	pContext = shader->GetContext();
//	LoadAndBuildMeshFromOBJFile(filename, shader);
//}
D3D11Mesh::D3D11Mesh(const char* file, ShaderProgramBase * shader)
{
	D3D11ShaderProgram* s = dynamic_cast<D3D11ShaderProgram*>(shader);
	pContext = s->GetContext();
	std::string filename(file);
	std::wstring newfile((int)filename.size(), 0);
	MultiByteToWideChar(CP_UTF8, 0, &filename[0], (int)filename.size(), &newfile[0], (int)filename.size());
	LoadAndBuildMeshFromOBJFile(newfile.c_str(), s);
}

D3D11Mesh::~D3D11Mesh()
{
	if (m_vertexLayout) m_vertexLayout->Release();

	if (m_vertexBuffer) m_vertexBuffer->Release();
}

void D3D11Mesh::Render()
{
	/*ID3D11DeviceContext* pContext = m_shader->GetContext();
	ID3D11ShaderResourceView* ptexRv = dynamic_cast<D3D11Texture*>(m_tex)->m_textureRV;*/

	unsigned int stride = sizeof(OGLVertex);
	unsigned int offset = 0;

	RHI::GetD3DContext()->IASetInputLayout(m_vertexLayout);
	RHI::GetD3DContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	RHI::GetD3DContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//pContext->PSSetShaderResources(0, 1, &ptexRv);

	RHI::GetD3DContext()->Draw(m_numtriangles*3, 0);
}
void D3D11Mesh::Render(ID3D11DeviceContext* pContext)
{
	/*ID3D11DeviceContext* pContext = m_shader->GetContext();
	ID3D11ShaderResourceView* ptexRv = dynamic_cast<D3D11Texture*>(m_tex)->m_textureRV;*/

	unsigned int stride = sizeof(OGLVertex);
	unsigned int offset = 0;

	pContext->IASetInputLayout(m_vertexLayout);
	pContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//pContext->PSSetShaderResources(0, 1, &ptexRv);

	pContext->Draw(m_numtriangles * 3, 0);
}
void D3D11Mesh::LoadAndBuildMeshFromOBJFile(LPCWSTR filename, D3D11ShaderProgram* shader)
{
	Triangle* mesh;

	m_numtriangles = importOBJMesh(filename, &mesh);

	m_shader = shader;

	unsigned int offset = sizeof(glm::vec3);

	D3D11_INPUT_ELEMENT_DESC vertlayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 2*offset, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	
	ID3D11Device* pDevice = shader->GetDevice();
	
	ID3DBlob* pVSBlob = shader->GetVSBlob();

	HRESULT hr = RHI::GetD3DDevice()->CreateInputLayout(vertlayout, ARRAYSIZE(vertlayout), pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_vertexLayout);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(OGLVertex)*m_numtriangles*3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &(mesh[0].m_vertices[0].m_position[0]);

	RHI::GetD3DDevice()->CreateBuffer(&bd, &initData, &m_vertexBuffer);

	delete [] mesh;
}
#endif