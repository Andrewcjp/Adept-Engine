#include "Source/Core/Stdafx.h"
#include "TestGameHud.h"
#include "UI/UIManager.h"
#include "UI/Core/UIBox.h"
#include "UI/GameUI/GameHud.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/Core/UILabel.h"
#include "TestGameGameMode.h"
#include "Components/TestPlayer.h"


TestGameHud::TestGameHud()
{}


TestGameHud::~TestGameHud()
{}

void TestGameHud::OnStart()
{
	Mode = (TestGameGameMode*)gameMode;
	ammoCounter = new UILabel("0/0", 0, 0, 0, 0);
	ammoCounter->SetScaled(0.2f, 0.2f);
	Context->AddWidget(ammoCounter);
}

void TestGameHud::OnUpdate()
{
	if (Mode->GetPlayer() != nullptr)
	{
		TestPlayer* player = Mode->GetPlayer()->GetComponent<TestPlayer>();
		if (player != nullptr)
		{
			ammoCounter->SetText(player->GetInfoString());
		}
	}
}

void TestGameHud::OnDestory()
{

}
