#pragma once

class UIImage;
class EditorUI
{
public:
	EditorUI();
	~EditorUI();
	void Init();
	void SetViewPortSize();
	UIImage* ViewPortImage = nullptr;
};

