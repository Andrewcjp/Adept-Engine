#include "Triangle.h"

Triangle::Triangle()
{
	
}

Triangle::Triangle(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 pos3)
{
	SetVertices(pos1, pos2, pos3);
}


Triangle::~Triangle()
{
}

void Triangle::SetVertices(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
{
	m_vertices[0].m_position = v0;
	m_vertices[1].m_position = v1;
	m_vertices[2].m_position = v2;
}

void Triangle::SetNormals(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2)
{
	m_vertices[0].m_normal = n0;
	m_vertices[1].m_normal = n1;
	m_vertices[2].m_normal = n2;
}
void Triangle::SetTangent(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2) {
	m_vertices[0].m_tangent = n0;
	m_vertices[1].m_tangent = n1;
	m_vertices[2].m_tangent = n2;
}
void Triangle::SetBiTangent(glm::vec3 n0, glm::vec3 n1, glm::vec3 n2)
{
	m_vertices[0].m_bitangent = n0;
	m_vertices[1].m_bitangent = n1;
	m_vertices[2].m_bitangent = n2;
}
void Triangle::SetTexCoords(glm::vec3 t0, glm::vec3 t1, glm::vec3 t2)
{
	m_vertices[0].m_texcoords = t0;
	m_vertices[1].m_texcoords = t1;
	m_vertices[2].m_texcoords = t2;
}