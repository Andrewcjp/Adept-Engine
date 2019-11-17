#pragma once
#include "UI\Core\UIWidget.h"
class UIImage : public UIWidget
{
public:
	UIImage(int w, int h, int x = 0, int y = 0);
	~UIImage();

	virtual void Render() override;
	virtual void UpdateScaled() override;

	void OnGatherBatches(UIRenderBatch * Groupbatchptr);
	BaseTextureRef TargetTexture;
	FrameBuffer* RenderTarget = nullptr;
};

