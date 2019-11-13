#pragma once
#include "../BasicWidgets/UIBox.h"
#include "../Core/CollisionRect.h"
#include <functional>
class UILabel;
class UIButton :
	public UIBox
{
public:
	CORE_API UIButton(int w, int h, int x = 0, int y = 0);
	virtual ~UIButton();
	void Render()override;
	UILabel * GetLabel();
	void MouseMove(int x, int y) override;
	bool MouseClick(int x, int y) override;
	void ResizeView(int w, int h, int x, int y) override;
	void UpdateScaled() override;
	void BindTarget(std::function<void()> t)
	{
		Target = t;
	}
	CORE_API void SetText(std::string t);
	glm::vec3 NormalColour = glm::vec3(0.5f);
	glm::vec3 Hovercolour = glm::vec3(1, 0, 0);
	glm::vec3 SelectedColour = glm::vec3(0, 0, 0.5f);
	void SetSelected(bool state);
	virtual void OnOwnerSet(UIWidgetContext* wc) override;

private:
	bool IsActiveSelect = false;
	bool WasSelected = false;
	std::string Labelstring = "Button";

	CollisionRect Rect;
	std::function<void()> Target;
	UILabel* Label = nullptr;

};

