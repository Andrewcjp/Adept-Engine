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

private:
	bool WasSelected = false;
	char* MText = "Button";
	CollisionRect Rect;
	std::function<void()> Target;
	UILabel* Label;
};

