#pragma once
#include "UIBox.h"
#include "CollisionRect.h"
#include <functional>
class UILabel;
class UIButton :
	public UIBox
{
public:
	UIButton(int w, int h, int x = 0, int y = 0);
	virtual ~UIButton();
	void Render()override;
	UILabel * GetLabel();
	void MouseMove(int x, int y) override;
	void MouseClick(int x, int y) override;
	void ResizeView(int w, int h, int x, int y) override;
	void UpdateScaled() override;
	void BindTarget(std::function<void()> t)
	{
		Target = t;
	}
	void SetText(std::string t);
	glm::vec3 NormalColour = glm::vec3(0.5f);
	glm::vec3 Hovercolour  = glm::vec3(1, 0, 0);
	glm::vec3 SelectedColour = glm::vec3(0, 0, 0.5f);
	void SetSelected(bool state);
private:
	bool IsActiveSelect = false;
	bool WasSelected = false;
	std::string Labelstring = "Button";

	CollisionRect Rect;
	std::function<void()> Target;
	UILabel* Label = nullptr;                   

};

