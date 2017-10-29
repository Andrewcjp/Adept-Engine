#pragma once
class GameObject;
class DebugLineDrawer;
class EditorGizmos
{
public:
	EditorGizmos();
	~EditorGizmos();
	void RenderGizmos(DebugLineDrawer * lien);
	void MouseDown(int x, int y);
	void MouseUp(int x, int y);
	void SetTarget(GameObject* t);
private:
	GameObject* target;
	float HandleLength = 10;

};

