#pragma once
#include "Core/CollisionRect.h"
#include "Core/EngineTypes.h"
#include <functional>
#define UISTATS 0
#define EDITORUI WITH_EDITOR
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
class UIWidgetContext;
class UIDropDown;
class UIImage;
class EditorUI;
class UILayoutManager;
class EditorOutliner;
class UIManager
{
public:
	static UIManager* instance;
	CORE_API static UIManager* Get();
	UIManager();
	UIManager(int w, int h);
	void InitCommonUI();
#if WITH_EDITOR
	void InitEditorUI();
	void SetFullscreen(bool state);
#endif
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
	void InitGameobjectList(std::vector<GameObject*>& gos);
	void UpdateGameObjectList(std::vector<GameObject*>& gos);
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
	void AddWidgetContext(UIWidgetContext*c);
	void RemoveWidgetContext(UIWidgetContext*c);
	static UIWidgetContext* GetDefaultContext();
	std::vector<UIWidgetContext*>& GetContexts()
	{
		return Contexts;
	}
	void SetEditorViewPortRenderTarget(FrameBuffer* target);
	bool IsFullScreen() const { return FullScreen; }
private:
	EditorUI* EditUI = nullptr;
	UIBox* TOP = nullptr;
	std::vector<UIWidgetContext*> Contexts;
	UIDropDown * DropdownCurrent = nullptr;
	std::vector<UIWidget*> WidgetsToRemove;//todo: use queue? and handle large deletes?
	static UIWidget* CurrentContext;
	float BottomHeight = 0.2f;
	float TopHeight = 0.2f;
	float RightWidth = 0.15f;
	float LeftWidth = 0.15f;
	int m_width = 0;
	int m_height = 0;
	UIBox* bottom = nullptr;
	float LastHeight = 0;
	float YHeight = 25;
	float XSpacing = 25;
	CollisionRect ViewportRect;
	glm::ivec4 ViewportArea;
	UIPopoutbox* testbox = nullptr;
	UIAssetManager* AssetMan;
	bool Blocking = false;
	Inspector* inspector = nullptr;
	std::vector<GameObject*>* GameObjectsPtr;
	bool FullScreen = false;
	UILayoutManager* EditorLayout = nullptr;
	EditorOutliner* OutLiner = nullptr;
};

