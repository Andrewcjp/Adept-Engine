#pragma once
#include <string>
#include <memory>
#include "glm/fwd.hpp"
#include <vector>
#include "CollisionRect.h"

class TextRenderer;
class UIWidget;
class GameObject;
class UIListBox;
class UIBox;
class LineDrawer;
class UIDrawBatcher;
class UIGraph;
class Inspector;
class UIManager
{
public:
	static UIManager* instance;
	UIManager();
	UIManager(int w, int h);
	void Test();
	~UIManager();
	void Initalise(int width, int height);
	Inspector * GetInspector();
	void RenderTextToScreen(int id, std::string text);
	void RenderTextToScreen(int id, std::string text, glm::vec3 colour);
	void RenderTextToScreen(std::string text, float x, float y, float scale, glm::vec3 colour);
	void UpdateSize(int width, int height);
	void AddWidget(UIWidget* widget);
	static void UpdateBatches();
	void UpdateWidgets();
	void RenderWidgets();
	void MouseMove(int x, int y);
	void MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	void InitGameobjectList(std::vector<GameObject*>* gos);
	void UpdateGameObjectList(std::vector<GameObject*>* gos);
	void RefreshGameObjectList();
	int GetWidth()
	{
		return m_width;
	}
	int GetHeight()
	{ 
		return m_height;
	}
	static int GetScaledWidth(float PC)
	{
		return (int)rint(PC *(instance->GetWidth()));
	}
	static int GetScaledHeight(float PC)
	{
		return (int)rint(PC *(instance->GetHeight()));
	}
	UIGraph* Graph;
	bool IsUIBlocking()
	{
		return Blocking;
	}
private:
	float BottomHeight = 0.2f;
	float TopHeight = 0.2f;
	float RightWidth = 0.15f;
	float LeftWidth = 0.15f;
	int m_width = 0;
	int m_height = 0;
	UIBox* bottom;
	std::unique_ptr<TextRenderer> textrender;
	std::vector<UIWidget*> widgets;
	UIListBox* box;
	float LastHeight = 0;
	float YHeight = 25;
	float XSpacing = 25;
	LineDrawer* LineBatcher;
	UIDrawBatcher* DrawBatcher;
	CollisionRect ViewportRect;
	bool Blocking = false;
	Inspector* inspector;
	std::vector<GameObject*>* GameObjectsPtr;
};

