#pragma once
#include "UIWidget.h"
class OGLShaderProgram;
class UILabel :
	public UIWidget
{
public:
	CORE_API UILabel(std::string  text, int w, int h, int x, int y);
	virtual ~UILabel();

	// Inherited via UIWidget
	virtual void Render() override;
	virtual void ResizeView(int w, int h, int x = 0, int y = 0) override;
	void SetText(std::string  text)
	{
		MText = text;
	}
	std::string GetText()
	{
		return MText;
	}
	float TextScale = 0.5f;
private:
	OGLShaderProgram* m_TextShader;
	std::string  MText = "";
};

