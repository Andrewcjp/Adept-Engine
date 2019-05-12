#pragma once
#include "UI/Core/CollisionRect.h"
#include "RHI/BaseTexture.h"

class UIWidget;
class DebugLineDrawer;
class UIDrawBatcher;
class TextRenderer;
class Shader_TexturedUI;
class BaseTexture;
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
	CORE_API int GetWidth();
	CORE_API int GetHeight();
	int GetScaledHeight(float PC);
	void UpdateSize(int width, int height, int Xoffset, int yoffset);
	void CleanUpWidgets();
	CORE_API void UpdateBatches();
	void Initalise(int width, int height);
	void RenderWidgets();
	void MarkRenderStateDirty();
	void MouseMove(int x, int y);
	void MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	UIDrawBatcher* GetBatcher() const;
	CORE_API DebugLineDrawer* GetLineBatcher()const;
	void SetOffset(glm::ivec2 newoff);
	void RenderTextToScreen(int id, std::string text);
	void RenderTextToScreen(int id, std::string text, glm::vec3 colour);
	void RenderTextToScreen(std::string text, float x, float y, float scale, glm::vec3 colour);
	glm::ivec2 Offset = glm::ivec2(0);
	void SetEnabled(bool state);
	bool GetEnabled() const
	{
		return Enabled;
	}
	CORE_API void DisplayPause();
	CORE_API void DisplayLoadingScreen();
	CORE_API void HideScreen();
	bool ShowQuadPostUI = false;
	bool ShowQuadpreUI = false;
private:
	bool Enabled = true;
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
	Shader_TexturedUI* Quad = nullptr;
	BaseTextureRef LoadingTex = nullptr;
	BaseTextureRef PauseTex = nullptr;
};

