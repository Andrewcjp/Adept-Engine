#pragma once
#include <vector>
#include "OpenGL\OGLShaderProgram.h"
struct UIVertex
{
	glm::vec2 position;
	int UseBackgound;
	glm::vec3 FrontColour;
	glm::vec3 BackColour;
};
class UIDrawBatcher
{
public:
	static UIDrawBatcher* instance;
	UIDrawBatcher();
	~UIDrawBatcher();
	void SendToGPU();
	void RenderBatches();
	void AddVertex(glm::vec2 pos, bool Back, glm::vec3 frontcol = glm::vec3(1,1,1), glm::vec3 backcol = glm::vec3(0));
	void ClearVertArray();
private:
	std::vector<UIVertex> BatchedVerts;
	GLuint	quad_vertexbuffer;
	OGLShaderProgram * m_TextShader;
};

