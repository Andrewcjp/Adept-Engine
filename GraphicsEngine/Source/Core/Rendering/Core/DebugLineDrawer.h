#pragma once

#include "Core/EngineTypes.h"
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
	DebugLineDrawer(bool DOnly = false);
	~DebugLineDrawer();
	void GenerateLines();
	void RenderLines();
	void ReallocBuffer(int NewSize);
	void RenderLines(glm::mat4& matrix);
	void ClearLines();

	void AddLine(glm::vec3 Start, glm::vec3 end, glm::vec3 colour, float time = 0);
	void OnResize(int newwidth, int newheight);

private:
	std::vector<WLine> Lines;
	class Shader_Line* LineShader = nullptr;
	class RHIBuffer* DataBuffer = nullptr;
	RHIBuffer* VertexBuffer = nullptr;
	class RHICommandList* CmdList = nullptr;
	glm::mat4 Projection;
	size_t VertsOnGPU = 0;
	struct VERTEX
	{
		glm::vec3 pos;
		glm::vec3 colour;
	};
	const int maxSize = 100000;
	int CurrentMaxVerts = 100;
	bool Is2DOnly = false;
};

