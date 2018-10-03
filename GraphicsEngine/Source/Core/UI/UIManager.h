#pragma once
#include <string>
#include <memory>
#include "glm/fwd.hpp"
#include <vector>
#include "UI/Core/CollisionRect.h"
#include <functional>
#include "Core/EngineTypes.h"
#define UISTATS 0
class TextRenderer;
class UIWidget;
class GameObject;
class UIListBox;
class UIBox;
class DebugLineDrawer;
class UIDrawBatcher;
class UIGraph;
class UIPopoutbox;
class Inspector;
class UIAssetManager;
class UIManager
{
public:
	static UIManager* instance;

	UIManager();
	UIManager(int w, int h);
	void InitEditorUI();
	void CreateDropDown(std::vector<std::string>& options, float width, float height, float x, float y, std::function<void(int)> Callback);
	void AlertBox(std::string MSg);
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
	void RenderWidgetText();
	void MouseMove(int x, int y);
	void MouseClick(int x, int y);
	void MouseClickUp(int x, int y);
	void InitGameobjectList(std::vector<GameObject*>* gos);
	void UpdateGameObjectList(std::vector<GameObject*>* gos);
	static void SelectedCallback(int i);
	void RefreshGameObjectList();
	int GetWidth();
	int GetHeight();
	static int GetScaledWidth(float PC);
	static int GetScaledHeight(float PC);
	UIGraph* Graph;
	bool IsUIBlocking();
	static UIWidget* GetCurrentContext();;
	static void SetCurrentcontext(UIWidget* widget);
	void RemoveWidget(UIWidget * widget);
	void CleanUpWidgets();
	static void CloseDropDown();
	IntRect GetEditorRect();
private:
	UIWidget * DropdownCurrent = nullptr;
	std::vector<UIWidget*> WidgetsToRemove;//todo: use queue? and handle large deletes?
	static UIWidget* CurrentContext;
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
	DebugLineDrawer* LineBatcher;
	UIDrawBatcher* DrawBatcher;
	CollisionRect ViewportRect;
	UIPopoutbox* testbox;
	UIAssetManager* AssetManager;
	bool Blocking = false;
	Inspector* inspector;
	std::vector<GameObject*>* GameObjectsPtr;
};

