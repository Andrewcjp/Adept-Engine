#include "UIWidgetContext.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Stdafx.h"
#include "UIDrawBatcher.h"
#include "UIWidget.h"
#include "Core/Platform/PlatformCore.h"
#include "UI/UIManager.h"
#include "Rendering/Shaders/Shader_TexturedUI.h"
#include "Core/Assets/AssetManager.h"
#include "RHI/BaseTexture.h"
UIWidgetContext::UIWidgetContext()
{
	Quad = new Shader_TexturedUI(RHI::GetDefaultDevice());
	TextureImportSettings set;
	set.ForceMipCount = 1;
	PauseTex = AssetManager::DirectLoadTextureAsset("texture\\UI\\PauseScreen.png");
	LoadingTex = AssetManager::DirectLoadTextureAsset("texture\\UI\\Loading screen.png", set);
	LoadingTex->MaxMip = 1;
	LoadingTex->UpdateSRV();
	Quad->Texture = PauseTex;
}

UIWidgetContext::~UIWidgetContext()
{
	SafeDelete(Quad);
	SafeDelete(TextRender);
	SafeDelete(LineBatcher);
	SafeDelete(DrawBatcher);
	MemoryUtils::DeleteVector(widgets);
}

void UIWidgetContext::AddWidget(UIWidget * widget)
{
	widget->SetOwner(this);
	widgets.push_back(widget);
	MarkRenderStateDirty();
}

void UIWidgetContext::UpdateWidgets()
{
	for (int i = 0; i < widgets.size(); i++)
	{
		widgets[i]->UpdateData();
	}
	CleanUpWidgets();
}
void UIWidgetContext::RenderWidgetText()
{
	return;
	TextRender->Reset();
	for (int i = 0; i < widgets.size(); i++)
	{
		if (widgets[i]->GetEnabled())
		{
			widgets[i]->Render();
		}
	}
	TextRender->Finish();
	if (ShowQuadPostUI)
	{
		Quad->Render();
	}
}

void UIWidgetContext::RemoveWidget(UIWidget* widget)
{
	widget->IsPendingKill = true;
	WidgetsToRemove.push_back(widget);
}

int UIWidgetContext::GetScaledWidth(float PC)
{
	return (int)rint(PC *(GetWidth()));
}

int UIWidgetContext::GetWidth()
{
	return m_width;
}

int UIWidgetContext::GetHeight()
{
	return m_height;
}

int UIWidgetContext::GetScaledHeight(float PC)
{
	return (int)rint(PC *(GetHeight()));
}

void UIWidgetContext::UpdateSize(int width, int height, int Xoffset, int yoffset)
{
	LineBatcher->OnResize(width, height);
	DrawBatcher->ClearVertArray();
	SetOffset(glm::ivec2(Xoffset, yoffset));
	m_width = width;
	m_height = height;
	ViewportRect = CollisionRect(GetScaledWidth(0.60f), GetScaledHeight(0.60f), GetScaledWidth(1), GetScaledHeight(1));

	struct less_than_key
	{
		bool operator() (UIWidget* struct1, UIWidget* struct2)
		{
			return (struct1->Priority > struct2->Priority);
		}
	};

	std::sort(widgets.begin(), widgets.end(), less_than_key());

	for (int i = (int)widgets.size() - 1; i >= 0; i--)
	{
		widgets[i]->UpdateScaled();
	}

	TextRender->UpdateSize(UIManager::instance->GetWidth(), UIManager::instance->GetHeight(), Offset);
	DrawBatcher->SendToGPU();
}

void UIWidgetContext::CleanUpWidgets()
{
	if (WidgetsToRemove.empty())
	{
		return;
	}
	for (int x = 0; x < widgets.size(); x++)
	{
		for (int i = 0; i < WidgetsToRemove.size(); i++)
		{
			if (widgets[x] == WidgetsToRemove[i])//todo: performance of this?
			{
				widgets.erase(widgets.begin() + x);
				SafeDelete(widgets[i]);
				break;
			}
		}
	}
	WidgetsToRemove.clear();
	MarkRenderStateDirty();
}

void UIWidgetContext::UpdateBatches()
{
	UpdateSize(m_width, m_height, DrawBatcher->Offset.x, DrawBatcher->Offset.y);
}

void UIWidgetContext::Initalise(int width, int height)
{
	ensure((width > 0 && height > 0));
	m_width = width;
	m_height = height;
	TextRender = new TextRenderer(m_width, m_height);
	TextRender->UpdateSize(width, height);
	DrawBatcher = new UIDrawBatcher();
	LineBatcher = new DebugLineDrawer(true);
}

void UIWidgetContext::RenderWidgets()
{
	if (RenderStateDirty || true)
	{
		UpdateBatches();
		RenderStateDirty = false;
	}
	if (ShowQuadpreUI)
	{
		Quad->Render();
	}
	//todo: move to not run every frame?
	DrawBatcher->RenderBatches();

#if UISTATS
	PerfManager::StartTimer("Line");
#endif
	LineBatcher->GenerateLines();
	LineBatcher->RenderLines();
#if UISTATS
	PerfManager::EndTimer("Line");
#endif
	//todo: GC?

}

void UIWidgetContext::MarkRenderStateDirty()
{
	RenderStateDirty = true;
}

void UIWidgetContext::MouseMove(int x, int y)
{
	for (int i = 0; i < widgets.size(); i++)
	{
		if (!widgets[i]->IsPendingKill)
		{
			widgets[i]->MouseMove(x, y);
		}
	}
}

void UIWidgetContext::MouseClick(int x, int y)
{
	for (int i = 0; i < widgets.size(); i++)
	{
		if (!widgets[i]->IsPendingKill)
		{
			widgets[i]->MouseClick(x, y);
		}
	}
}

void UIWidgetContext::MouseClickUp(int x, int y)
{
	for (int i = 0; i < widgets.size(); i++)
	{
		if (!widgets[i]->IsPendingKill)
		{
			widgets[i]->MouseClickUp(x, y);
		}
	}
}

UIDrawBatcher* UIWidgetContext::GetBatcher() const
{
	return DrawBatcher;
}

DebugLineDrawer * UIWidgetContext::GetLineBatcher() const
{
	return LineBatcher;
}

void UIWidgetContext::SetOffset(glm::ivec2 newoff)
{
	GetBatcher()->Offset = newoff;
	Offset = newoff;
}

void UIWidgetContext::RenderTextToScreen(int id, std::string text)
{
	RenderTextToScreen(id, text, glm::vec3(0.6, 1.0f, 0.2f));
}

void UIWidgetContext::RenderTextToScreen(int id, std::string text, glm::vec3 colour)
{
	RenderTextToScreen(text, XSpacing, static_cast<float>(m_height - (YHeight*id)), 0.5f, colour);
}

void UIWidgetContext::RenderTextToScreen(std::string text, float x, float y, float scale, glm::vec3 colour)
{
	TextRender->RenderFromAtlas(text, x, y, scale, colour, false);
}

void UIWidgetContext::SetEnabled(bool state)
{
	Enabled = state;
	UpdateBatches();
}

void UIWidgetContext::DisplayPause()
{
	HideScreen();
	ShowQuadpreUI = true;
	Quad->Texture = PauseTex;
	Quad->blend = true;
}

void UIWidgetContext::DisplayLoadingScreen()
{
	HideScreen();
	ShowQuadPostUI = true;
	Quad->Texture = LoadingTex;
	Quad->blend = false;
}

void UIWidgetContext::HideScreen()
{
	ShowQuadPostUI = false;
	ShowQuadpreUI = false;
}
