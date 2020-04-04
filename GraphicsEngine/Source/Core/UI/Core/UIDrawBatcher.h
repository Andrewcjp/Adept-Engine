#pragma once

#include "RHI/RHICommandList.h"

struct TextBatch;
struct UIVertex
{
	glm::vec2 position;
	int UseBackgound;
	glm::vec3 FrontColour;
	glm::vec3 BackColour;
};
namespace ERenderBatchType
{
	enum Type
	{
		Verts,
		TexturedVerts,
		Limit
	};
}
//contains all the needed data to render a batch of widgets
struct UIRenderBatch
{
	std::vector<UIVertex> Verts;
	void AddVertex(glm::vec2 pos, bool Back, glm::vec3 frontcol, glm::vec3 backcol);
	ERenderBatchType::Type BatchType = ERenderBatchType::Verts;
	std::string text = "";
	glm::vec2 pos = glm::vec2();
	void AddText(std::string text, glm::vec2 pos,float scale = 0.3f,glm::vec3 colour = glm::vec3(1,1,1));
	std::vector<TextBatch*> TextData;
	BaseTextureRef TextureInUse;
	FrameBuffer* RenderTarget = nullptr;
};
class UIDrawBatcher
{
public:

	UIDrawBatcher();
	void Init();
	void SetState(RHICommandList * list, UIRenderBatch* batch);
	void ReallocBuffer(int NewSize);
	~UIDrawBatcher();
	void SendToGPU();
	void RenderBatches(RHICommandList* List);
	void Render(RHICommandList * list);
	void ClearVertArray();
	void CleanUp();
	glm::ivec2 Offset = glm::ivec2(0);
	RHICommandList* commandlist = nullptr;
	void AddBatch(UIRenderBatch* Batch);
	void BuildBatches();
private:
	class Shader_UIBatch* Shader;
	int UIMin = 300;
	std::vector<UIVertex> BatchedVerts;
	RHIBuffer* VertexBuffer = nullptr;

	const int Max_Verts = 10000;
	int Current_Max_Verts = UIMin;
	std::vector<UIRenderBatch*> Batches;
};

