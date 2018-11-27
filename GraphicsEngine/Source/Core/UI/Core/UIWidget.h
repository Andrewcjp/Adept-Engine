#pragma once

class UIWidgetContext;

typedef unsigned long long UINT_PTR;
#define USE_BATCHED_DRAW 1
class UIWidget
{
public:
	struct Aligment
	{
		float SizeMax = 0.05f;
		//float SizeMin = 0.05f;
	};

	Aligment AligmentStruct;
	static glm::vec3 DefaultColour;
	CORE_API UIWidget(int w, int h, int x = 0, int y = 0);
	~UIWidget();
	virtual void Render() = 0;
	virtual void ResizeView(int w, int h, int x = 0, int y =  0);
	CORE_API void SetScaled(float Width, float height, float xoff = 0.0f, float yoff = 0.0f);
	virtual void UpdateScaled();
	void SetOwner(UIWidgetContext* wc);
	glm::vec3 Colour;
	virtual void UpdateData();
	virtual void MouseMove(int x, int y);
	virtual bool MouseClick(int x, int y);
	virtual void MouseClickUp(int x, int y);
	virtual void PostBatchRender();
	int mwidth;
	int mheight;
	int X;
	int Y;
	virtual void ProcessKeyDown(UINT_PTR key);
	void ProcessKeyUp(UINT_PTR);
	void SetEnabled(bool state);
	bool GetEnabled()
	{
		return IsActive;
	}
	int Priority = 0;
	bool operator<(UIWidget* that) const;
	void InvalidateRenderstate();

	float WidthScale = 0;
	float HeightScale = 0;
	float XoffsetScale = 0;
	float YoffsetScale = 0;
	bool IsPendingKill = false;
protected:
	float TextDefaultScale = 0.3f;
	bool IsActive = true;
	UIWidgetContext* OwningContext = nullptr;
private:
	//Is this widget Pixel Indepent?
	bool UseScaled = false;

};

