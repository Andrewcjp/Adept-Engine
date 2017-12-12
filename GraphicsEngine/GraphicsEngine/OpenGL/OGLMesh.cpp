#include <Windows.h>
#include "OGLMesh.h"
#include "Rendering/Core/Triangle.h"
#include "../Core/Assets/OBJFileReader.h"
#include "OGLTexture.h"
#include <GLEW\GL\glew.h>
#include <iostream>
#include <string>
OGLMesh::OGLMesh()
{

}

OGLMesh::OGLMesh(LPCWSTR filename)
{
	
	LoadAndBuildMeshFromOBJFile(filename);
}

OGLMesh::OGLMesh(std::string filename,const char* name)
{
	AssetName = name;
	std::wstring newfile((int)filename.size(),0);
	MultiByteToWideChar(CP_UTF8, 0, &filename[0], (int)filename.size(), &newfile[0], (int)filename.size());
	LoadAndBuildMeshFromOBJFile(newfile.c_str());
}

OGLMesh::~OGLMesh()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo_verts);
}

void OGLMesh::Render(CommandListDef*)
{

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	glDrawArrays(GL_TRIANGLES, 0, m_numtriangles * 3);

	glBindVertexArray(0);
}
void OGLMesh::RenderInstance(int number)
{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	//glVertexAttribDivisor(5, 1);
	//glVertexAttribDivisor(6, 1);
	//glVertexAttribDivisor(7, 1);
	glDrawArraysInstanced(GL_TRIANGLES, 0, m_numtriangles * 3, number);

	glBindVertexArray(0);
}

void OGLMesh::LoadAndBuildMeshFromOBJFile(LPCWSTR filename)
{
	Triangle* mesh;

	m_numtriangles = importOBJMesh(filename, &mesh);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo_verts);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	glBufferData(GL_ARRAY_BUFFER, 3 * m_numtriangles * sizeof(OGLVertex), &(mesh[0].m_vertices[0].m_position[0]), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), 0);
	glEnableVertexAttribArray(0);
	
	//'type cast': conversion from 'int' to 'void *' of greater size

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + 3 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + 4 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);

	delete[] mesh;
	std::cout << "Loaded Mesh " << std::endl;
}