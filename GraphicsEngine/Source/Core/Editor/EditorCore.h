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
private:
	GameObject* SelectedObject = nullptr;
};

#endif