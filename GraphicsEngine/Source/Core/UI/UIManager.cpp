#include "UIManager.h"
#include "Core/GameObject.h"
#include "Core/UIDrawBatcher.h"
#include "CompoundWidgets/UIPopoutbox.h"
#include "Core/UIWidgetContext.h"
#include "Core/Utils/VectorUtils.h"
#include "Editor/EditorCore.h"
#include "Editor/EditorWindow.h"
#include "EditorUI/EditorUI.h"
#include "EditorUI/UIAssetManager.h"
#include "CompoundWidgets/DebugConsole.h"
#include "CompoundWidgets/UIDropDown.h"
#include "CompoundWidgets/UIGraph.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "BasicWidgets/UIImage.h"
#include "Core/Assets/AssetManager.h"
#include "CompoundWidgets/UIWindow.h"
#include "CompoundWidgets/UITab.h"
#include "EditorUI/UILayoutManager.h"

UIManager* UIManager::instance = nullptr;
UIWidget* UIManager::CurrentContext = nullptr;
UIManager * UIManager::Get()
{
	return instance;
}
UIManager::UIManager()
{}

UIManager::UIManager(int w, int h)
{
	instance = this;
	BottomHeight = 0.2f;
	TopHeight = 0.1f;
	RightWidth = 0.2f;
	LeftWidth = 0.2f;
	m_width = w;
	m_height = h;
	EditUI = new EditorUI();
	Initalise(w, h);

	InitCommonUI();
#if EDITORUI
	InitEditorUI();
#endif
}
void UIManager::InitCommonUI()
{
	UIGraph* graph = new UIGraph(Contexts[0]->GetLineBatcher(), 250, 150, 15, 25);
	Graph = graph;
	AddWidget(graph);
	graph->SetEnabled(false);
	DebugConsole* wid = new DebugConsole(100, 100, 100, 100);
	wid->SetAbsoluteSize(1920, 40, 0, 40);
	AddWidget(wid);
}
#if WITH_EDITOR
void UIManager::InitEditorUI()
{
	EditUI->ViewPortImage = new UIImage(0, 0, 0, 0);
	ViewportArea = glm::ivec4(1920, 1080, 0, 0);
	SetFullscreen(false);
	AddWidget(EditUI->ViewPortImage);
	EditUI->SetViewPortSize();
	//return;
#if 0
	BottomHeight = 0.25f;
	TOP = new UIBox(m_width, GetScaledHeight(0.2f), 0, 0);
	TOP->SetScaled(1.0f, TopHeight, 0.0f, 1.0f - TopHeight);
	const int size = 100;
	TOP->SetRootSpaceScaled(0, size, 0, size);
	TOP->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	TOP->GetTransfrom()->SetStretchMode(EAxisStretch::Width);
	AddWidget(TOP);



	TOP->IgnoreboundsCheck = true;

	UIButton* button = new UIButton(200, 50, 0, 500);
	button->SetScaled(0.05f, 0.075f, 0.5f - 0.05f, 0);
	button->BindTarget(std::bind(&EditorWindow::EnterPlayMode, EditorWindow::GetInstance()));
	button->SetText("Play");
	button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Bottom);
	TOP->AddChild(button);
	AddWidget(button);
	button = new UIButton(200, 50, 0, 500);
	button->SetScaled(0.05f, 0.075f, 0.5f, 0);
	button->BindTarget(std::bind(&EditorWindow::ExitPlayMode, EditorWindow::GetInstance()));
	button->SetText("Stop");
	button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Bottom);
	TOP->AddChild(button);
	AddWidget(button);
	button = new UIButton(200, 50, 0, 500);
	button->SetScaled(0.05f, 0.075f, 0.5f + 0.05f, 0);
	button->BindTarget(std::bind(&EditorWindow::Eject, EditorWindow::GetInstance()));
	button->SetText("Eject");
	button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Bottom);
	TOP->AddChild(button);
	AddWidget(button);
	AssetMan = new UIAssetManager();
	AssetMan->SetRootSpaceSize(1920, 200, 0, 0);
	AddWidget(AssetMan);
#endif

	EditorLayout = new UILayoutManager();
	UIWindow* WindoTest = new UIWindow();
	WindoTest->GetTransfrom()->SetScalingMode(EWidetSizeSpace::RootSpace);
	WindoTest->GetTransfrom()->SetPos(IntPoint(500, 200));
	WindoTest->GetTransfrom()->SetSize(IntPoint(500, 800));
	WindoTest->UpdateSize();
	AddWidget(WindoTest);

	UITab* tab = new UITab();
	WindoTest->AddTab(tab);
	tab = new UITab();

	UIWindow* RightWin = new UIWindow();
	RightWin->AddTab(new UITab());
	RightWin->AddTab(new UITab());
	AddWidget(RightWin);
	UIWindow* Bottomw = new UIWindow();
	Bottomw->AddTab(new UITab());
	Bottomw->AddTab(new UITab());
	AddWidget(Bottomw);

	UIWindow* TopWin = new UIWindow();
	TopWin->AddTab(new UITab());
	AddWidget(TopWin);

	EditorLayout->SetWidget(UILayoutManager::Left, WindoTest);
	EditorLayout->SetWidget(UILayoutManager::Right, RightWin);
	EditorLayout->SetWidget(UILayoutManager::Bottom, Bottomw);
	EditorLayout->SetWidget(UILayoutManager::Top, TopWin);
	EditorLayout->SetWidget(UILayoutManager::Centre, EditUI->ViewPortImage);


	inspector = new Inspector(0, 0, 0, 0);
	inspector->SetRootSpaceScaled(0, 0, 0, 0);
	WindoTest->AddTab(inspector);
	const int Small = GetScaledWidth(0.2);
	ViewportArea = glm::ivec4(1920 - Small, 1080 - Small, GetScaledWidth(0.2f), Small);
	ViewportRect = CollisionRect(ViewportArea.x, ViewportArea.x, ViewportArea.z, ViewportArea.w);
	UpdateBatches();
}

void UIManager::SetFullscreen(bool state)
{
	if (state)
	{
		EditUI->ViewPortImage->SetRootSpaceSize(1920, 1080, 0, 0);
	}
	else
	{
		EditUI->ViewPortImage->SetRootSpaceSize(ViewportArea.x, ViewportArea.y, ViewportArea.z, ViewportArea.w);
	}
	EditUI->SetViewPortSize();
	FullScreen = state;
}

#endif
void UIManager::CreateDropDown(std::vector<std::string> &options, float width, float height, float x, float y, std::function<void(int)> Callback)
{
	UIDropDown * testbox3 = new UIDropDown(100, 300, 250, 150); //new UIDropDown(100, 300, x, y);
	testbox3->Priority = 10;
	testbox3->SetRootSpaceSize(width, height * 2, x, y);

	for (int i = 0; i < options.size(); i++)
	{
		testbox3->AddItem(options[i]);
	}
	if (Callback)
	{
		testbox3->SelectionChanged = Callback;
	}
	AddWidget(testbox3);
	testbox3->UpdateScaled();
	instance->DropdownCurrent = testbox3;
}

void UIManager::AlertBox(std::string MSg)
{
	UIPopoutbox* testboxx = new UIPopoutbox(100, 300, 250, 150);
	testboxx->SetScaled(RightWidth, TopHeight * 2, 0.5f - (RightWidth / 2), 0.5f - (TopHeight * 2 / 2));
	testboxx->SetText(MSg);
	AddWidget(testboxx);
	UIManager::UpdateBatches();
}

UIManager::~UIManager()
{
	MemoryUtils::DeleteVector(Contexts);
	SafeDelete(TextRenderer::instance);//todo: improve ownership
}

void UIManager::Initalise(int width, int height)
{
	m_width = width;
	m_height = height;
	new TextRenderer(width, height, true);
}

Inspector* UIManager::GetInspector()
{
	return inspector;
}

void UIManager::RenderTextToScreen(int id, std::string text)
{
	RenderTextToScreen(id, text, glm::vec3(0.6, 1.0f, 0.2f));
}

void UIManager::RenderTextToScreen(int id, std::string text, glm::vec3 colour)
{
	RenderTextToScreen(text, XSpacing, static_cast<float>(m_height - (YHeight*id)), 0.5f, colour);
}

void UIManager::RenderTextToScreen(std::string text, float x, float y, float scale, glm::vec3 colour)
{
	TextRenderer::instance->RenderFromAtlas(text, x, y, scale, colour, false);
}

void UIManager::UpdateSize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (TextRenderer::instance)
	{
		TextRenderer::instance->UpdateSize(width, height);
	}

#if EDITORUI
	for (int i = 0; i < Contexts.size(); i++)
	{
		if (i > 0)
		{
			Contexts[i]->UpdateSize(GetScaledWidth(0.60f), GetScaledHeight(0.60f), GetScaledWidth(RightWidth), GetScaledWidth(TopHeight));
		}
		else
		{
			Contexts[i]->UpdateSize(width, height, 0, 0);
		}
	}
	if (EditUI != nullptr)
	{
		SetFullscreen(IsFullScreen());

	}
#else
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->UpdateSize(width, height, 0, 0);
	}
#endif
}

void UIManager::AddWidget(UIWidget * widget)
{
	Contexts[0]->AddWidget(widget);
}

void UIManager::UpdateBatches()
{
	for (int i = 0; i < instance->Contexts.size(); i++)
	{
		instance->Contexts[i]->MarkRenderStateDirty();
	}
}

void UIManager::UpdateWidgets()
{
	EditorLayout->Update();
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->UpdateWidgets();
	}
}

void UIManager::RenderWidgets()
{
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->RenderWidgets();
	}
}

void UIManager::RenderWidgetText()
{
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->RenderWidgetText();
	}
}

//todo: prevent issue with adding while itoring!
void UIManager::MouseMove(int x, int y)
{
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->MouseMove(x, y);
	}

}

void UIManager::MouseClick(int x, int y)
{
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->MouseClick(x, y);
	}
}

void UIManager::MouseClickUp(int x, int y)
{
	for (int i = 0; i < Contexts.size(); i++)
	{
		Contexts[i]->MouseClickUp(x, y);
	}
}

void UIManager::InitGameobjectList(std::vector<GameObject*>& gos)
{
#if EDITORUI
	GameObjectsPtr = &gos;
	box = new UIListBox(GetScaledWidth(0.15f), GetScaledHeight(.8f), 0, GetScaledHeight(0.2f));
	AddWidget(box);
	RefreshGameObjectList();
#endif
}

void UIManager::UpdateGameObjectList(std::vector<GameObject*>& gos)
{
	GameObjectsPtr = &gos;
}

void UIManager::SelectedCallback(int i)
{
#if EDITORUI
	if (instance)
	{
		EditorWindow::GetEditorCore()->SetSelectedObjectIndex(i);
	}
#endif
}

void UIManager::RefreshGameObjectList()
{
#if EDITORUI
	if (box != nullptr && GameObjectsPtr != nullptr)
	{
		box->RemoveAll();
		box->SetScaled(LeftWidth, 1.0f - (BottomHeight + TopHeight), 0.0, BottomHeight);
		using std::placeholders::_1;
		box->SelectionChanged = std::bind(&UIManager::SelectedCallback, _1);
		for (int i = 0; i < (*(GameObjectsPtr)).size(); i++)
		{
			(*GameObjectsPtr)[i]->PostChangeProperties();
			box->AddItem((*(GameObjectsPtr))[i]->GetName().c_str());
		}
		if (GetInspector())
		{
			GetInspector()->Refresh();
		}
		if ((*GameObjectsPtr).size())
		{
			SelectedCallback(2);
		}
		UpdateBatches();
	}
#endif
}

int UIManager::GetWidth()
{
	return m_width;
}

int UIManager::GetHeight()
{
	return m_height;
}

int UIManager::GetScaledWidth(float PC)
{
	return (int)rint(PC *(instance->GetWidth()));
}

int UIManager::GetScaledHeight(float PC)
{
	return (int)rint(PC *(instance->GetHeight()));
}

bool UIManager::IsUIBlocking()
{
	return Blocking;
}

UIWidget * UIManager::GetCurrentContext()
{
	if (instance != nullptr)
	{
		return instance->CurrentContext;
	}
	return nullptr;
}

void UIManager::SetCurrentcontext(UIWidget * widget)
{
	if (instance != nullptr)
	{
		instance->CurrentContext = widget;
	}
}

void UIManager::RemoveWidget(UIWidget* widget)
{
	widget->IsPendingKill = true;
	WidgetsToRemove.push_back(widget);
}

void UIManager::CleanUpWidgets()
{
	//if (WidgetsToRemove.empty())
	//{
	//	return;
	//}
	//for (int x = 0; x < widgets.size(); x++)
	//{
	//	for (int i = 0; i < WidgetsToRemove.size(); i++)
	//	{
	//		if (widgets[x] == WidgetsToRemove[i])//todo: performance of this?
	//		{
	//			widgets.erase(widgets.begin() + x);
	//			SafeDelete(widgets[i]);
	//			break;
	//		}
	//	}
	//}
	//WidgetsToRemove.clear();
	//UpdateBatches();
}

void UIManager::CloseDropDown()
{
	if (instance != nullptr && instance->DropdownCurrent != nullptr && instance->DropdownCurrent->frameCreated < RHI::GetFrameCount())
	{
		instance->DropdownCurrent->GetOwningContext()->RemoveWidget(instance->DropdownCurrent);
		instance->DropdownCurrent = nullptr;
	}
}

IntRect UIManager::GetEditorRect()
{
	IntRect rect;
	rect.Min.x = GetScaledWidth(LeftWidth);
	rect.Min.y = GetScaledHeight(TopHeight);

	rect.Max.x = GetScaledWidth(0.8f - RightWidth);
	rect.Max.y = GetScaledHeight(0.9f - BottomHeight);
	return rect;
}

void UIManager::AddWidgetContext(UIWidgetContext * c)
{
	c->Initalise(m_width, m_height);
	Contexts.push_back(c);
	UpdateSize(m_width, m_height);
}

void UIManager::RemoveWidgetContext(UIWidgetContext * c)
{
	VectorUtils::Remove(Contexts, c);
}

UIWidgetContext * UIManager::GetDefaultContext()
{
	return instance->Contexts[0];
}

void UIManager::SetEditorViewPortRenderTarget(FrameBuffer * target)
{
	EditUI->ViewPortImage->RenderTarget = target;
}

