#pragma once

#include <include/assimp/Importer.hpp>
#include <include/assimp/scene.h>
#include <include/assimp/postprocess.h>
#include <iostream>
#include <vector>
#include "Vertex.h"
#include <GLEW\GL\glew.h>
#include "Renderable.h"
class Mesh :public Renderable
{
public:
	Mesh();
	Mesh(std::string filename);
	~Mesh();
	void Render() override;
	void Init(Vertex * verts, int vertsize, int * Indicies, int indexsize);
	void LoadMeshFromFile(std::string filename);
private:
	GLuint Indexbuffer;
	GLuint vertexbuffer;
	GLuint VertexArrayID;
	Vertex* m_verts;
	int* m_Indicies;
	int m_vertsize;
	int m_indexsize;
};

