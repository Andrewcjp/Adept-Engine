#pragma once
#include <vector>
#include <map>
#include <GLEW\GL\glew.h>
#include "glm/fwd.hpp"
typedef struct _LineItem
{
	glm::vec2 startpos;
	glm::vec2 endpos;
	glm::vec3 colour;
	float Thickness;
}Line;
class OGLShaderProgram;
class LineDrawer
{
public:
	LineDrawer();
	~LineDrawer();
	void GenerateLines();
	void RenderLines();
	void ClearLines();

	void AddLine(glm::vec2 Start, glm::vec2 end,glm::vec3 colour , float thickness = 0);
private:
	GLuint	quad_vertexbuffer;
	OGLShaderProgram * m_TextShader;
	
#if 0
	std::vector<Line> Lines;
	std::vector<GLfloat> Verts;
#else
	Line* Lines;
	GLfloat* Verts;
#endif
	size_t VertsOnGPU = 0;
	size_t LineCount = 0;
	size_t CurrentAllocateLines = 50;
	int vertindex = 0;
};

