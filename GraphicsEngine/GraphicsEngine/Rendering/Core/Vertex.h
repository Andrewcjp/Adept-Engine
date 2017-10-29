#pragma once

#include <include/glm/glm.hpp>

class Vertex
{
public:
	Vertex() {

	}
	Vertex(const glm::vec3& pos, const glm::vec2& texcoord, const glm::vec3& normal, const glm::vec3& tangent) {
		this->pos = pos;
		this->texCoord = texcoord;
		this->normal = normal;
		this->tangent = tangent;		
	}
	inline glm::vec3* GetPos() { return &pos; }
	inline glm::vec2* GetTexCoord() { return &texCoord; }
	inline glm::vec3* Getnormal() { return &normal; }
	inline glm::vec3* Gettangent() { return &tangent; }
	inline glm::vec3* Getbutangent() { return &bitangent; }
private:
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;

};

