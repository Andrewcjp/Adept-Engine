#pragma once
#include "EngineGlobals.h"
#if BUILD_D3D11
#include <d3d11_1.h>
#include "D3D11\D3D11Shader.h"
#include "../Rendering/Core/Renderable.h"
//#include "Listener.h"

class D3D11Plane :public Renderable
{
private:
	struct SimpleVertex
	{
		float position[3];
	};

private:
	D3D11ShaderProgram*			m_shader;
	ID3D11InputLayout*			m_vertexLayout;
	ID3D11Buffer*				m_vertexBuffer;
	ID3D11Buffer*				m_indexBuffer;

	void			InitUnitCube(D3D11ShaderProgram* shader);

public:
	D3D11Plane(D3D11ShaderProgram* shader);
	~D3D11Plane();

	void			Render(CommandListDef* list = nullptr);
};

#endif