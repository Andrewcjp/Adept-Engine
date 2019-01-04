#pragma once
class GameObject;
class DebugLineDrawer;
#if WITH_EDITOR
namespace CurrentGizmoMode
{
	enum type
	{
		Translate,
		Rotate,
		Scale,
		Limit
	};
};
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
	void SetTarget(GameObject* t);
	void SwitchMode(int index);
private:
	GameObject* target = nullptr;
	float HandleLength = 10;
	int YStartPos = 0;
	int XStartPos = 0;
	float Scale = 0.1f;
	glm::vec3 StartPos;
	bool DidFirst = false;
	int ModeIndex = 0;
	/*bool */

};
#endif
