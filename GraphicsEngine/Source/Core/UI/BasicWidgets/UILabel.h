#pragma once
#include "../Core/UIWidget.h"
class UILabel :
	public UIWidget
{
public:
	CORE_API UILabel(std::string  text, int w, int h, int x, int y);
	virtual ~UILabel();

	// Inherited via UIWidget
	virtual void Render() override;
	virtual void ResizeView(int w, int h, int x = 0, int y = 0) override;
	CORE_API void SetText(const std::string& text);
	CORE_API std::string GetText();
	glm::ivec2 CalulateTextSize();
	float TextScale = 0.5f;

	virtual void OnGatherBatches(UIRenderBatch* Groupbatchptr = nullptr) override;

private:
	std::string  MText = "";
};

