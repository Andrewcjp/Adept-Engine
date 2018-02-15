#pragma once
class GameObject;
class DebugLineDrawer;
class EditorGizmos
{
public:
	enum Axis
	{
		AxisZ,
		AxisX,
		AxisY
	};
	EditorGizmos();
	~EditorGizmos();
	void UpdateAxis(float amt, Axis axis);
	void UpdateAxis(float amt);

	void Update(float deltatime);
	void RenderGizmos(DebugLineDrawer * lien);
	void MouseDown(int x, int y);
	void MouseUp(int x, int y);
	void SetTarget(GameObject* t);
private:
	GameObject* target;
	float HandleLength = 10;
	int YStartPos = 0;
	float Scale = 0.1f;
	glm::vec3 StartPos;
	bool DidFirst = false;
	/*bool */

};

