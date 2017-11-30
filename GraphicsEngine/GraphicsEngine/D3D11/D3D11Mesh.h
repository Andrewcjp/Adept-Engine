#pragma once
#include "EngineGlobals.h"
#if BUILD_D3D11
#include <Windows.h>
#include "../Rendering/Core/Renderable.h"
#include "Rendering/Core/Triangle.h"
#include "D3D11Shader.h"
#include <d3d11_1.h>

class D3D11Mesh :	public Renderable
{
	private: 
		D3D11ShaderProgram*			m_shader;
		ID3D11InputLayout*			m_vertexLayout;
		ID3D11Buffer*				m_vertexBuffer;
		ID3D11Buffer*				m_indexBuffer;
		int							m_numtriangles;
		ID3D11DeviceContext* pContext;
	public:
		D3D11Mesh();
	//	D3D11Mesh(LPCWSTR filename, D3D11ShaderProgram * shader);
		D3D11Mesh(const char* file, ShaderProgramBase * shader);
		~D3D11Mesh();

		void LoadAndBuildMeshFromOBJFile(LPCWSTR filename, D3D11ShaderProgram* shader);

		void Render(CommandListDef* list = nullptr);

		void Render(ID3D11DeviceContext * pContext);

};

#endif