#include "EditorGizmos.h"
#include "Core/GameObject.h"

#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Input/Input.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/Camera.h"
#include "Core/Utils/DebugDrawers.h"
#include "Rendering/Core/Screen.h"
#include "UI/UIManager.h"
#include "UI/EditorUI/EditorUI.h"
#include "UI/Core/UIWidget.h"
#include "UI/Core/UITransform.h"
#include "UI/BasicWidgets/UIImage.h"
#if WITH_EDITOR
EditorGizmos::EditorGizmos()
{
	HandleLength = 5;
	ModeIndex = CurrentGizmoMode::Translate;
}

EditorGizmos::~EditorGizmos()
{}

void EditorGizmos::UpdateAxis(float amt, Axis axis)
{
	if (ModeIndex == CurrentGizmoMode::Translate)
	{
		glm::vec3 newpos = target->GetPosition();
		switch (axis)
		{
		case AxisX:
			newpos.x += amt;
			break;
		case AxisY:
			newpos.y += amt;
			break;
		case AxisZ:
			newpos.z += amt;
			break;
		}
		target->ChangePos_editor(newpos);
	}
	else if (ModeIndex == CurrentGizmoMode::Rotate)
	{
		glm::vec3 newrot = target->GetTransform()->GetEulerRot();
		switch (axis)
		{
		case AxisX:
			newrot.x += amt;
			break;
		case AxisY:
			newrot.y += amt;
			break;
		case AxisZ:
			newrot.z += amt;
			break;
		}
		target->GetTransform()->SetQrot(glm::quat(glm::radians(newrot)));
	}
	target->PostChangeProperties();
}

//todo: axis rotations relative to view etc.
//todo: axis hit boxes?
void EditorGizmos::UpdateAxis(float amt)
{

	if (Input::GetKey(KeyCode::X))
	{
		UpdateAxis(amt, AxisY);
	}
	else if (Input::GetKey(KeyCode::Z))
	{
		UpdateAxis(amt, AxisX);
	}
	else if (Input::GetKey(KeyCode::C))
	{
		UpdateAxis(amt, AxisZ);
	}

}
void EditorGizmos::Update(float deltatime)
{
	TickNew();
	if (Input::GetKey(KeyCode::X) || Input::GetKey(KeyCode::Z) || Input::GetKey(KeyCode::C))
	{
		if (target != nullptr && DidFirst)
		{
			if (Input::GetKey(KeyCode::Z))
			{
				UpdateAxis((float)(XStartPos - Input::GetMousePos().x)*Scale);
			}
			else
			{
				UpdateAxis((float)(YStartPos - Input::GetMousePos().y)*Scale);
			}

		}
		YStartPos = (int)Input::GetMousePos().y;
		XStartPos = (int)Input::GetMousePos().x;
		DidFirst = true;
	}
	else
	{
		DidFirst = false;
	}
}
void EditorGizmos::RenderGizmos(DebugLineDrawer* lien)
{
	if (target == nullptr)
	{
		return;
	}
	glm::vec3 Xaxisend = target->GetTransform()->GetPos() + glm::vec3(HandleLength, 0, 0);
	glm::vec3 yaxisend = target->GetTransform()->GetPos() + glm::vec3(0, HandleLength, 0);
	glm::vec3 zaxisend = target->GetTransform()->GetPos() + glm::vec3(0, 0, HandleLength);
	if (lien != nullptr)
	{
		lien->AddLine(target->GetTransform()->GetPos(), Xaxisend, (HighLightAxis == Handle_X || HighLightAxis == Handle_ALL) ? glm::vec3(1, 1, 1) : glm::vec3(1, 0, 0));
		lien->AddLine(target->GetTransform()->GetPos(), yaxisend, (HighLightAxis == Handle_Y || HighLightAxis == Handle_ALL) ? glm::vec3(1, 1, 1) : glm::vec3(0, 1, 0));
		lien->AddLine(target->GetTransform()->GetPos(), zaxisend, (HighLightAxis == Handle_Z || HighLightAxis == Handle_ALL) ? glm::vec3(1, 1, 1) : glm::vec3(0, 0, 1));
	}
}

bool EditorGizmos::intersectPlane(Plane & p, const Ray &ray, float &t)
{
	float denom = glm::dot(p.normal, ray.direction);
	if (abs(denom) > 0.0001f) // your favorite epsilon
	{
		t = glm::dot((p.position - ray.origin), (p.normal)) / denom;
		if (t >= 0) return true; // you might want to allow an epsilon here too
	}
	return false;
}
float GetMaxComponent(glm::vec3 v)
{
	float out = v.x;
	out = glm::max(v.y, out);
	out = glm::max(v.z, out);
	return out;
}
void EditorGizmos::TickNew()
{
	if (target == nullptr)
	{
		return;
	}
	InitBoxes();
	glm::vec3 TargetPos = target->GetTransform()->GetPos();
	Ray r;
	r.t = 0.0f;
	glm::vec2 pos = Screen::GetWindowRes() - glm::ivec2(Input::GetMousePos().x, Input::GetMousePos().y);
#if WITH_EDITOR
	glm::vec4 Offset = UIManager::Get()->EditUI->ViewPortImage->GetTransfrom()->GetTransfromRect();
	glm::vec2 EdgeOffset = glm::vec2(Offset.z, Offset.w) - (glm::vec2(Offset.x, Offset.y) / 2.0f);
	pos = glm::vec2(Screen::GetWindowRes()) - glm::vec2(Input::GetMousePos().x, Input::GetMousePos().y);
	pos += glm::vec2(Offset.z / 2.0f, -Offset.w);
#endif
	SceneRenderer::Get()->GetCurrentCamera()->GetRayAtScreenPos(pos.x, pos.y, r.direction, r.origin);
	Log::LogTextToScreen(glm::to_string(pos));
	//DebugLineDrawer::Get()->AddLine(r.origin, r.direction * 1000, glm::vec3(1), 10);
	CurrentWidget LastAxis = CurrentAxis;
	if (CurrentAxis == Limit)
	{
		HighLightAxis = CurrentWidget::Limit;
		for (int i = 0; i < CurrentWidget::Limit; i++)
		{
			if (Boxes[i].intersect(r))
			{
				CurrentAxis = (CurrentWidget)i;
				HighLightAxis = CurrentAxis;
				break;
			}
		}
	}
	glm::vec3 ProjectPoint = glm::vec3(0);
	if (Input::GetMouseButton(MouseButton::ButtonLeft))
	{
		if (CurrentAxis != Limit)
		{
			glm::vec3 AxisLock = glm::vec3(CurrentAxis == Handle_X, CurrentAxis == Handle_Y, CurrentAxis == Handle_Z);
			if (CurrentAxis == Handle_ALL)
			{
				AxisLock = glm::vec3(1, 1, 1);
			}
			Planes[CurrentAxis].position = TargetPos;
			intersectPlane(Planes[CurrentAxis], r, r.t);
			ProjectPoint = r.origin + (r.direction*r.t);
			if (LastAxis != CurrentAxis)
			{
				LastPos = ProjectPoint;
			}
			glm::vec3 delta = (ProjectPoint - LastPos);
			delta *= AxisLock;
			if (ModeIndex == CurrentGizmoMode::Translate)
			{
				glm::vec3 o = target->GetTransform()->GetPos();
				target->ChangePos_editor(o + delta);
			}
			else if (ModeIndex == CurrentGizmoMode::Scale)
			{
				if (CurrentAxis == Handle_ALL)
				{
					//lock to uniform scale
					delta = glm::vec3(delta.x, delta.x, delta.x);
				}
				glm::vec3 o = target->GetTransform()->GetScale();
				target->GetTransform()->SetScale(o + delta);
			}
		}
	}
	else
	{
		CurrentAxis = Limit;
	}
	LastPos = ProjectPoint;
}
void EditorGizmos::SetTarget(GameObject * t)
{
	target = t;
}
void EditorGizmos::SwitchMode(int index)
{
	ModeIndex = index;
}
void EditorGizmos::InitBoxes()
{
	if (target == nullptr)
	{
		return;
	}
	glm::vec3 TargetPos = target->GetTransform()->GetPos();
	const float Length = 4;
	const float BoxSize = 1;

	Boxes[CurrentWidget::Handle_X].Min = TargetPos - glm::vec3(0, BoxSize, BoxSize);
	Boxes[CurrentWidget::Handle_X].Max = TargetPos + glm::vec3(Length, BoxSize, BoxSize);
	Planes[CurrentWidget::Handle_X].normal = glm::vec3(0, 1, 0);

	Boxes[CurrentWidget::Handle_Y].Min = TargetPos - glm::vec3(BoxSize, 0, BoxSize);
	Boxes[CurrentWidget::Handle_Y].Max = TargetPos + glm::vec3(BoxSize, Length, BoxSize);
	Planes[CurrentWidget::Handle_Y].normal = glm::vec3(1, 0, 0);


	Boxes[CurrentWidget::Handle_Z].Min = TargetPos - glm::vec3(BoxSize, BoxSize, 0);
	Boxes[CurrentWidget::Handle_Z].Max = TargetPos + glm::vec3(BoxSize, BoxSize, Length);
	Planes[CurrentWidget::Handle_Z].normal = glm::vec3(0, 1, 0);

	Boxes[CurrentWidget::Handle_ALL].Min = TargetPos - glm::vec3(BoxSize, BoxSize, BoxSize);
	Boxes[CurrentWidget::Handle_ALL].Max = TargetPos + glm::vec3(BoxSize, BoxSize, BoxSize);
	Planes[CurrentWidget::Handle_ALL].normal = glm::vec3(0, 1, 0);
}

bool EditorGizmos::Box::intersect(const Ray &r)
{
	float tmin = (Min.x - r.origin.x) / r.direction.x;
	float tmax = (Max.x - r.origin.x) / r.direction.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (Min.y - r.origin.y) / r.direction.y;
	float tymax = (Max.y - r.origin.y) / r.direction.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (Min.z - r.origin.z) / r.direction.z;
	float tzmax = (Max.z - r.origin.z) / r.direction.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	return true;
}
#endif