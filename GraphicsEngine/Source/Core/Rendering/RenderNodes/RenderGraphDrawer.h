#pragma once

class RenderGraph;
class RenderNode;
class DebugLineDrawer;
//debug drawer for the render graph
class RenderGraphDrawer
{
public:
	RenderGraphDrawer();
	~RenderGraphDrawer();
	void Draw(RenderGraph* G);
	void Update();
	void DrawNode(RenderNode * Node, glm::vec3 & LastPos, int index);
	void DrawLinks(RenderNode * A, int index, glm::vec3 & LastPos);

	glm::vec3 GetPosOfNodeindex(int index);

	std::string CreateLinksforNode(RenderNode * B);

	void WriteGraphViz(RenderGraph* G);

private:
	DebugLineDrawer* drawer = nullptr;
	float NodeSpaceing = 40;
	glm::vec3 nodesize = glm::vec3(50, 100, 0);
	float InputSpaceing = 20;
	const glm::vec3 Colour = glm::vec3(1, 1, 1);
	glm::vec3 StartPos = glm::vec3(0, 400, 0);
	RenderGraph* CurrentGraph = nullptr;
	int LinkId = 0;
};

