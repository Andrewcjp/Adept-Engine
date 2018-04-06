#pragma once
#include "UIWidget.h"
#include <GLEW\GL\glew.h>
#include "CollisionRect.h"
class OGLShaderProgram;
class BaseTexture;
class UIBox :
	public UIWidget
{
public:
	UIBox(int w, int h, int x = 0, int y = 0);
	
	virtual void ResizeView(int w, int h, int x, int y) override;
	~UIBox();
	void MouseMove(int x ,int y) override;
	void MouseClick(int x, int y) override;
	void MouseClickUp(int x, int y) override;
	virtual void Render() override;
	glm::vec3 BackgoundColour = glm::vec3(0);
protected:
	void Init();
	
private:
	bool Resizeable = false;
	int WidthOfCollisonRects = 30;//todo: ScreenSpace relative
	bool Selected = false;
	CollisionRect RightRect;
	GLuint	quad_vertexbuffer;
	OGLShaderProgram * m_TextShader;
	BaseTexture* texture = nullptr;
	float EdgeGap = 2.0f;
};

