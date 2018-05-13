#pragma once
#include <vector>
#include <map>
#include "glm/fwd.hpp"
typedef struct _LineItem
{
	glm::vec2 startpos;
	glm::vec2 endpos;
	glm::vec3 colour;
	float Thickness;
}Line;
class ShaderProgramBase;
class LineDrawer
{
public:
	LineDrawer();
	void InitOGL();
	~LineDrawer();
	void GenerateLines();
	void RenderLines();
	void RenderLines_OpenGL();
	void ClearLines();

	void AddLine(glm::vec2 Start, glm::vec2 end,glm::vec3 colour , float thickness = 0);
private:
	int	quad_vertexbuffer;
	ShaderProgramBase * m_TextShader;
	Line* Lines;
	float* Verts;

	size_t VertsOnGPU = 0;
	size_t LineCount = 0;
	size_t CurrentAllocateLines = 50;
	int vertindex = 0;
};

