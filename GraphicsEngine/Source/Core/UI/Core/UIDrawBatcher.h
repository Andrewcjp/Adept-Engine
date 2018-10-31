#pragma once

#include "RHI/RHICommandList.h"
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
	void Init();
	void ReallocBuffer(int NewSize);
	~UIDrawBatcher();
	void SendToGPU();
	void RenderBatches();
	void Render(RHICommandList * list);
	void AddVertex(glm::vec2 pos, bool Back, glm::vec3 frontcol = glm::vec3(1,1,1), glm::vec3 backcol = glm::vec3(0));
	void ClearVertArray();
	void CleanUp();
private:	
	class Shader_UIBatch* Shader;
	int UIMin = 300;
	std::vector<UIVertex> BatchedVerts;
	RHIBuffer* VertexBuffer = nullptr;
	RHICommandList* commandlist = nullptr;
	const int Max_Verts = 10000;
	int Current_Max_Verts = UIMin;
};

