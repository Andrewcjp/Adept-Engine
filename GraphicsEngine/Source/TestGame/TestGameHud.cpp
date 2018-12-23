#include "Source/Core/Stdafx.h"
#include "TestGameHud.h"
#include "UI/UIManager.h"
#include "UI/Core/UIBox.h"
#include "UI/GameUI/GameHud.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/Core/UILabel.h"
#include "TestGameGameMode.h"
#include "Components/TestPlayer.h"
#include "Rendering/Core/DebugLineDrawer.h"


TestGameHud::TestGameHud()
{}


TestGameHud::~TestGameHud()
{}

void TestGameHud::OnStart()
{
	Mode = (TestGameGameMode*)gameMode;
	ammoCounter = new UILabel("0/0", 0, 0, 0, 0);
	ammoCounter->SetScaled(0.2f, 0.2f);
	ammoCounter->TextScale = 0.45f;
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
		const float Crosshairsize = 30.0f;
		const float CentreExclude = 7.0f;
		glm::vec3 ScreenCentre = glm::vec3(Context->Offset, 0) + glm::vec3(Context->GetWidth() / 2, Context->GetHeight() / 2, 0);
		Context->GetLineBatcher()->AddLine(glm::vec3(1, 0, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(1, 0, 0) * Crosshairsize, glm::vec3(1));
		Context->GetLineBatcher()->AddLine(glm::vec3(-1, 0, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(-1, 0, 0) * Crosshairsize, glm::vec3(1));
		Context->GetLineBatcher()->AddLine(glm::vec3(0, 1, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(0, 1, 0)*Crosshairsize, glm::vec3(1));
		Context->GetLineBatcher()->AddLine(glm::vec3(0, -1, 0)*CentreExclude + ScreenCentre, ScreenCentre + glm::vec3(0, -1, 0)*Crosshairsize, glm::vec3(1));
	}
}

void TestGameHud::OnDestory()
{

}
