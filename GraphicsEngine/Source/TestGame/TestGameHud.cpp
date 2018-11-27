#include "Source/Core/Stdafx.h"
#include "TestGameHud.h"
#include "UI/UIManager.h"
#include "UI/Core/UIBox.h"
#include "UI/GameUI/GameHud.h"
#include "UI/Core/UIWidgetContext.h"


TestGameHud::TestGameHud()
{}


TestGameHud::~TestGameHud()
{}

void TestGameHud::OnStart()
{
	UIBox* t = new UIBox(1200, 1200, 0, 0);
	t->SetScaled(0.2f, 0.2f);
	Context->AddWidget(t);
}

void TestGameHud::OnUpdate()
{

}

void TestGameHud::OnDestory()
{

}
