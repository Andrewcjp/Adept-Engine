#pragma once
#include <vector>
#include "OpenGL\OGLShaderProgram.h"
#include <d3d12.h>
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
	void InitOGL();
	void InitD3D12();
	void ReallocBuffer(int NewSize);
	~UIDrawBatcher();
	void SendToGPU();
	void SendToGPU_OpenGL();
	void SendToGPU_D3D12();
	void RenderBatches_OpenGL();
	void RenderBatches_D3D12();
	void Render(RHICommandList * list);
	void RenderBatches();
	void AddVertex(glm::vec2 pos, bool Back, glm::vec3 frontcol = glm::vec3(1,1,1), glm::vec3 backcol = glm::vec3(0));
	void ClearVertArray();
	void CleanUp();
private:
	
	GLuint	quad_vertexbuffer;

	OGLShaderProgram * m_TextShader;
	class Shader_UIBatch* Shader;

	int UIMin = 300;
	std::vector<UIVertex> BatchedVerts;
	RHIBuffer* VertexBuffer = nullptr;
	RHICommandList* commandlist = nullptr;

};

