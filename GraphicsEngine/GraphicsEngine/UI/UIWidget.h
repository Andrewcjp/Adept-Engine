#pragma once
#include "glm\glm.hpp"
#include <string>
typedef unsigned long long UINT_PTR;
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
	UIWidget(int w, int h, int x = 0, int y = 0);
	~UIWidget();
	virtual void Render() = 0;
	virtual void ResizeView(int w, int h, int x = 0, int y = 0);
	void SetScaled(float Width, float height, float xoff = 0.0f, float yoff = 0.0f);
	virtual void UpdateScaled();
	glm::vec3 Colour;
	virtual void UpdateData();
	virtual void MouseMove(int x, int y);
	virtual void MouseClick(int x, int y);
	virtual void MouseClickUp(int x, int y);
	virtual void PostBatchRender();
	int mwidth;
	int mheight;
	int X;
	int Y;
	virtual void ProcessKeyDown(UINT_PTR key);
	void SetEnabled(bool state)
	{
		IsActive = state;
	}
	bool GetEnabled()
	{
		return IsActive;
	}
protected:
	float WidthScale = 0;
	float HeightScale = 0;
	float XoffsetScale = 0;
	float YoffsetScale = 0;
	float TextDefaultScale = 0.3f;
	//glm::vec3 DefaultColour = glm::vec3(1);
	bool IsActive = true;
private:
	//Is this widget Pixel Indepent?
	bool UseScaled = false;
	
};

