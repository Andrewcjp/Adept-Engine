#pragma once
#include "UI/Core/CollisionRect.h"

class UIWidget;
class DebugLineDrawer;
class UIDrawBatcher;
class TextRenderer;
///Contains all the widgets and drawers for this HUD or editor
class UIWidgetContext
{
public:
	CORE_API UIWidgetContext();
	CORE_API ~UIWidgetContext();
	CORE_API void AddWidget(UIWidget * widget);
	void UpdateWidgets();
	void RenderWidgetText();
	CORE_API void RemoveWidget(UIWidget * widget);
	int GetScaledWidth(float PC);
	int GetWidth();
	int GetHeight();
	int GetScaledHeight(float PC);
	void UpdateSize(int width, int height, int Xoffset, int yoffset);
	void CleanUpWidgets();
	void UpdateBatches();
	void Initalise(int width, int height);
	void RenderWidgets();
	void MarkRenderStateDirty();
	void MouseMove(int x, int y);
	void MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	UIDrawBatcher* GetBatcher() const;
	DebugLineDrawer* GetLineBatcher()const;
	void SetOffset(glm::ivec2 newoff);
	void RenderTextToScreen(int id, std::string text);
	void RenderTextToScreen(int id, std::string text, glm::vec3 colour);
	void RenderTextToScreen(std::string text, float x, float y, float scale, glm::vec3 colour);
	glm::ivec2 Offset = glm::ivec2(0);
private:
	const float YHeight = 25;
	const float XSpacing = 25;
	int m_width = 0;
	int m_height = 0;
	CollisionRect ViewportRect;
	std::vector<UIWidget*> widgets;
	std::vector<UIWidget*> WidgetsToRemove;
	TextRenderer* TextRender = nullptr;
	DebugLineDrawer* LineBatcher = nullptr;
	UIDrawBatcher* DrawBatcher = nullptr;
	bool RenderStateDirty = true;
};

