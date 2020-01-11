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
	void TickNew();
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

	glm::vec3 LastPos = glm::vec3(0, 0, 0);

	struct  Ray
	{
		glm::vec3 direction;
		glm::vec3 origin;
		float t;
	};
	struct Box
	{
		glm::vec3 Max;
		glm::vec3 Min;
		bool intersect(const Ray &r);
	};
	struct Plane
	{
		glm::vec3 normal;
		glm::vec3 position;
	};
	void InitBoxes();
	enum CurrentWidget
	{
		Handle_ALL,
		Handle_X,
		Handle_Y,
		Handle_Z,
		Limit
	};
	bool intersectPlane(Plane & p, const Ray & ray, float & t);
	Box Boxes[CurrentWidget::Limit];
	Plane Planes[CurrentWidget::Limit];

	CurrentWidget CurrentAxis = CurrentWidget::Limit;
	CurrentWidget HighLightAxis = CurrentWidget::Limit;
};
#endif
