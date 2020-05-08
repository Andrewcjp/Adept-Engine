#pragma once
#include "../Core/UIWidget.h"
#include "../Core/CollisionRect.h"
class UIBox :
	public UIWidget
{
public:
	CORE_API UIBox(int w, int h, int x = 0, int y = 0);

	virtual void ResizeView(int w, int h, int x, int y) override;
	~UIBox();
	void MouseMove(int x, int y) override;
	bool MouseClick(int x, int y) override;
	void MouseClickUp(int x, int y) override;
	virtual void Render() override;
	glm::vec3 BackgoundColour = glm::vec3(0);

	virtual void OnGatherBatches(UIRenderBatch* Groupbatchptr = nullptr) override;

protected:
	void Init();
	bool SkipRender = false;
private:
	bool Resizeable = false;
	int WidthOfCollisonRects = 30;//todo: ScreenSpace relative
	bool Selected = false;
	CollisionRect RightRect;
	float EdgeGap = 2.0f;
	
};

