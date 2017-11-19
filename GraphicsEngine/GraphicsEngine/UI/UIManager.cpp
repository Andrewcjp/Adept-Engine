#include "UIManager.h"
#include "TextRenderer.h"
#include "glm\glm.hpp"
#include "UIWidget.h"
#include "UIBox.h"
#include "UIButton.h"
#include "UIListBox.h"
#include "UILabel.h"
#include "Core/GameObject.h"
#include "Core/Input.h"
#include "UI/UIEditField.h"
#include "LineDrawer.h"
#include "UIGraph.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Core/Performance/PerfManager.h"
#include "UIDrawBatcher.h"
#include "DebugConsole.h"
#include "../Editor/Inspector.h"
#include "../Editor/EditorWindow.h"
UIManager* UIManager::instance = nullptr;
UIManager::UIManager()
{
}

UIManager::UIManager(int w, int h)
{
	BottomHeight = 0.2f;
	TopHeight = 0.1f;
	RightWidth = 0.2f;
	LeftWidth = 0.2f;
	LineBatcher = new LineDrawer();
	DrawBatcher = new UIDrawBatcher();
	Initalise(w, h);
	instance = this;


	bottom = new UIBox(m_width, GetScaledHeight(0.2f), 0, 0);
	bottom->SetScaled(1.0f - RightWidth, BottomHeight);
	AddWidget(bottom);

	UIBox* TOP = new UIBox(m_width, GetScaledHeight(0.2f), 0, 0);
	TOP->SetScaled(1.0f, TopHeight, 0.0f, 1.0f - TopHeight);
	AddWidget(TOP);

	UIGraph* graph = new UIGraph(LineBatcher, 250, 150, 15, 25);
	Graph = graph;
	AddWidget(graph);

	inspector = new Inspector(m_width, GetScaledHeight(0.2f), 0, 0);
	inspector->SetScaled(RightWidth, 1.0f - (TopHeight), 1 - RightWidth);
	AddWidget(inspector);

	ViewportRect = CollisionRect(GetScaledWidth(0.70f), GetScaledHeight(0.70f), 0, 0);
	//b = new UIBox(GetScaledWidth(0.15), GetScaledHeight(0.85), w- GetScaledWidth(0.15), GetScaledHeight(0.15));
	//b->Colour = glm::vec3(0.6);
	//AddWidget(b);
	UIButton* button = new UIButton(200, 50, 0, 500);
	button->SetScaled(0.05f, 0.075f, 0.5f - 0.05f, 1.0f - (TopHeight));
	button->BindTarget(std::bind(&EditorWindow::EnterPlayMode, EditorWindow::GetInstance()));
	button->SetText("Play");
	AddWidget(button);
	button = new UIButton(200, 50, 0, 500);
	button->SetScaled(0.05f, 0.075f, 0.5f, 1.0f - (TopHeight));
	button->BindTarget(std::bind(&EditorWindow::ExitPlayMode, EditorWindow::GetInstance()));
	button->SetText("Stop ");
	AddWidget(button);
	DebugConsole* wid = new DebugConsole(100, 100, 100, 100);
	wid->SetScaled(1.0f, 0.05f, 0.0f, 0.3f);
	AddWidget(wid);

	//AddWidget(button);
}
void UIManager::Test()
{
	//__debugbreak();
}

UIManager::~UIManager()
{
	textrender.reset();
	for (int i = 0; i < widgets.size(); i++)
	{
		delete widgets[i];
	}
}

void UIManager::Initalise(int width, int height)
{
	m_width = width;
	m_height = height;
	textrender = std::make_unique<TextRenderer>(m_width, m_height);
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
	textrender->RenderFromAtlas(text, XSpacing, static_cast<float>(m_height - (YHeight*id)), 0.5f, colour);
}

void UIManager::RenderTextToScreen(std::string text, float x, float y, float scale, glm::vec3 colour)
{
	textrender->RenderFromAtlas(text, x, y, scale, colour);
}

void UIManager::UpdateSize(int width, int height)
{
	DrawBatcher->ClearVertArray();
	m_width = width;
	m_height = height;
	ViewportRect = CollisionRect(GetScaledWidth(0.70f), GetScaledHeight(0.70f), 0, 0);
	for (int i = 0; i < widgets.size(); i++)
	{
		//widgets[i]->SetWindowDim(m_width, m_height);
		widgets[i]->UpdateScaled();
	}
	//	box->UpdateScaled();
	//	box->ResizeView(GetScaledWidth(0.15f), GetScaledHeight(.6f), 0, GetScaledHeight(0.2f));
		/*bottom->ResizeView(m_width, GetScaledHeight(0.2), 0, 0);*/
	textrender->UpdateSize(width, height);
	DrawBatcher->SendToGPU();
}

void UIManager::AddWidget(UIWidget * widget)
{
	//widget->SetWindowDim(m_width, m_height);
	widgets.push_back(widget);
}
void UIManager::UpdateBatches()
{
	if (instance != nullptr)
	{
		instance->UpdateSize(instance->m_width, instance->m_height);
	}
}
void UIManager::RenderWidgets()
{

	//todo: move to not run every frame?
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->UpdateData();
	}
	DrawBatcher->RenderBatches();
	for (int i = 0; i < widgets.size(); i++)
	{
		if (widgets[i]->GetEnabled())
		{
			widgets[i]->Render();
		}
	}
	//	PerfManager::StartTimer("Line");
	LineBatcher->GenerateLines();
	LineBatcher->RenderLines();
	//	PerfManager::EndTimer("Line");


}

void UIManager::MouseMove(int x, int y)
{
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->MouseMove(x, y);
	}
	Blocking = !(ViewportRect.Contains(x, y));
}

void UIManager::MouseClick(int x, int y)
{
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->MouseClick(x, y);
	}
}
void UIManager::MouseClickUp(int x, int y)
{
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->MouseClickUp(x, y);
	}
}

void UIManager::InitGameobjectList(std::vector<GameObject*>*gos)
{
	GameObjectsPtr = gos;
	box = new UIListBox(GetScaledWidth(0.15f), GetScaledHeight(.8f), 0, GetScaledHeight(0.2f));
	AddWidget(box);
	RefreshGameObjectList();
}
void UIManager::UpdateGameObjectList(std::vector<GameObject*>*gos)
{
	GameObjectsPtr = gos;
}

void UIManager::RefreshGameObjectList()
{
	if (box != nullptr && GameObjectsPtr != nullptr)
	{
		box->RemoveAll();
		//	box->SetScaled(LeftWidth, 0.70f, 0.0f, 0.15f);
		box->SetScaled(LeftWidth, 1.0f - (BottomHeight + TopHeight), 0.0, BottomHeight);
		using std::placeholders::_1;
		box->SelectionChanged = std::bind(&Input::SetSelectedObject, _1);
		for (int i = 0; i < (*(GameObjectsPtr)).size(); i++)
		{
			box->AddItem((*(GameObjectsPtr))[i]->GetName().c_str());
		}
		UpdateBatches();
	}
}

