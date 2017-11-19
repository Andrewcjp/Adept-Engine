#pragma once

#include <Windows.h>
#include "../Rendering/Core/Renderable.h"
#include "Rendering/Core/Triangle.h"


class OGLMesh : public Renderable
{
private:
	unsigned int	m_vao;
	unsigned int	m_vbo_verts;
	int							m_numtriangles;

public:
	unsigned int GetVao() { return m_vao; }
	OGLMesh();
	OGLMesh(LPCWSTR filename);
	OGLMesh(std::string filename, const char * name);
	OGLMesh(std::string filename);
	~OGLMesh();

	void LoadAndBuildMeshFromOBJFile(LPCWSTR filename);

	void Render();

	void RenderInstance(int number);

};