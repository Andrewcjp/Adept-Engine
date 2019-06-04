#pragma once

#include "Core/EngineTypes.h"
typedef struct _WLineItem
{
	glm::vec3 startpos;
	glm::vec3 endpos;
	glm::vec3 colour;
	float Time;
	float Thickness;
	bool Persistent = false;
	bool NewLine = true;
}WLine;
class DebugLineDrawer
{
public:
	DebugLineDrawer(bool DOnly = false);
	~DebugLineDrawer();
	void GenerateLines();
	void UpdateLineBuffer(int offset);
	void CreateLineVerts(WLine & line);
	void RenderLines(FrameBuffer * Buffer, RHICommandList * list, EEye::Type eye);
	CORE_API void AddLine(glm::vec3 Start, glm::vec3 end, glm::vec3 colour, float time = 0);
	void OnResize(int newwidth, int newheight);
	void FlushDebugLines();
	CORE_API static DebugLineDrawer* Get();
	CORE_API static DebugLineDrawer* Get2();
private:
	void ReallocBuffer(int NewSize);

	void RegenerateVertBuffer();
	void ClearLines();
	static DebugLineDrawer* instance;
	static DebugLineDrawer* twodinstance;
	std::vector<WLine> Lines;
	class Shader_Line* LineShader = nullptr;
	class RHIBuffer* DataBuffer = nullptr;
	RHIBuffer* VertexBuffer = nullptr;
	//class RHICommandList* CmdList = nullptr;
	glm::mat4 Projection;
	size_t VertsOnGPU = 0;
	struct VERTEX
	{
		glm::vec3 pos;
		glm::vec3 colour;
	};
	bool RegenNeeded = true;
	std::vector<VERTEX> Verts;
	const int maxSize = 80000;
	int CurrentMaxVerts = 100;
	bool Is2DOnly = false;
	int EraseIndex = 0;
	VERTEX* VertArray = nullptr;
	int CurrentVertStreamLength = 10;
	void InsertVertex(VERTEX v);
	int InsertPtr = 0;
	void ResizeVertexStream(int newsize);
	void ResetVertexStream();
	int GetArraySize()
	{
		return InsertPtr;
	}
};

