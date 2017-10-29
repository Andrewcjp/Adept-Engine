#pragma once
#include "include\glm\glm.hpp"
#include "glm\glm.hpp"
#include <vector>

struct OGLVertex
{
	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec3 m_texcoords;
	glm::vec3 m_tangent;
	glm::vec3 m_bitangent;
};

class Triangle
{
public:
	OGLVertex		m_vertices[3];
public:
	Triangle();
	Triangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3);
	~Triangle();

	void SetVertices(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
	void SetNormals(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2);
	void SetTangent(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2);
	void SetBiTangent(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2);
	void SetTexCoords(glm::vec3 t0, glm::vec3 t1, glm::vec3 t2);
};

