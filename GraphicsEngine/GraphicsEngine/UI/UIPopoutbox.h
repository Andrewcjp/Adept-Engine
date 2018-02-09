#pragma once
#include "UIBox.h"
#include "UIButton.h"
class UIPopoutbox : public UIBox
{
public:
	UIPopoutbox(int w, int h, int x, int y);
	virtual ~UIPopoutbox();
	void Render() override;
	void ResizeView(int w, int h, int x, int y) override;
	void MouseMove(int x, int y) override;
	void MouseClick(int x, int y) override;
	void SetText(std::string text);
private:
	UIButton* OkayButton;
	UILabel* BodyLabel;
	void CloseBox();
};

