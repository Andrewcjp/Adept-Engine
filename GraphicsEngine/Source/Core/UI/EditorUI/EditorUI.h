#pragma once

class UIImage;
class EditorSettingsMenu;
class EditorUI
{
public:
	EditorUI();
	~EditorUI();
	void Init();
	void SetViewPortSize();
	static void HandleCMD_GameObject(int index);
	UIImage* ViewPortImage = nullptr;

	static void HandleCMD(int index);
private:
	static EditorUI* Instance;
	PlatformMenuBar FileMenu;
	PlatformMenuBar GameObjectMenu;
	EditorSettingsMenu* Settings;
	UIWidgetContext* context;
};

