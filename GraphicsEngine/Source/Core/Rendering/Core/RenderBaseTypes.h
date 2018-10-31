#pragma once
struct OGLVertex
{
	glm::vec3 m_position = glm::vec3(0);
	glm::vec3 m_normal = glm::vec3(0);
	glm::vec2 m_texcoords = glm::vec2(0);//todo: vec3?
	glm::vec3 m_tangent = glm::vec3(0);
	glm::vec3 m_bitangent = glm::vec3(0);
	OGLVertex() {}
	OGLVertex(const glm::vec3& pos, const glm::vec2& texcoord, const glm::vec3& normal, const glm::vec3& tangent)
	{
		m_position = pos;
		m_texcoords = (texcoord);
		m_normal = normal;
		m_tangent = tangent;
	}
};