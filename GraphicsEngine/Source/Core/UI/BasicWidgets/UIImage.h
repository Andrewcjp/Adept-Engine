#pragma once
#include "UI\Core\UIWidget.h"
class UIImage : public UIWidget
{
public:
	UIImage(int w, int h, int x = 0, int y = 0);
	~UIImage();

	void OnGatherBatches(UIRenderBatch * Groupbatchptr);
	BaseTextureRef TargetTexture;
	FrameBuffer* RenderTarget = nullptr;
};

