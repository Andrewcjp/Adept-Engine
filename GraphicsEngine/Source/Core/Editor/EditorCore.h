#pragma once
class GameObject;
#if WITH_EDITOR
class EditorCore
{
public:
	EditorCore();
	~EditorCore();
	void SetSelectedObjectIndex(int index);
	GameObject* GetSelectedObject();
	void SetSelectedObject(GameObject* newobj);
	bool LockAspect = true;
	float LockedAspect = 1920.0f / 1080.0f;
private:
	GameObject* SelectedObject = nullptr;
};

#endif