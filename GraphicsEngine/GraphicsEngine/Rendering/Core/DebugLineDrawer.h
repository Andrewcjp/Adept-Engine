#pragma once
#include <vector>
class OGLShaderProgram;
typedef struct _WLineItem
{
	glm::vec3 startpos;
	glm::vec3 endpos;
	glm::vec3 colour;
	float Time;
	float Thickness;
}WLine;
class DebugLineDrawer
{
public:
	static DebugLineDrawer* instance;
	DebugLineDrawer();
	~DebugLineDrawer();
	void GenerateLines();
	void RenderLines(glm::mat4 matrix);
	void ClearLines();

	void AddLine(glm::vec3 Start, glm::vec3 end, glm::vec3 colour, float thickness = 0);
private:
	std::vector<WLine> Lines;
	GLuint	quad_vertexbuffer;
	OGLShaderProgram * m_TextShader;
	size_t VertsOnGPU = 0;
};

