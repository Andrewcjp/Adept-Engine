#include "EditorGizmos.h"
#include "Core/GameObject.h"

#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Input/Input.h"
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

	if (Input::GetKey('x'))
	{
		UpdateAxis(amt, AxisY);
	}
	else if (Input::GetKey('z'))
	{
		UpdateAxis(amt, AxisX);
	}
	else if (Input::GetKey('c'))
	{
		UpdateAxis(amt, AxisZ);
	}

}
void EditorGizmos::Update(float deltatime)
{
	if (Input::GetKey('x') || Input::GetKey('z') || Input::GetKey('c'))
	{
		if (target != nullptr && DidFirst)
		{
			if (Input::GetKey('z'))
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
		lien->AddLine(target->GetTransform()->GetPos(), Xaxisend, glm::vec3(1, 0, 0));
		lien->AddLine(target->GetTransform()->GetPos(), yaxisend, glm::vec3(0, 1, 0));
		lien->AddLine(target->GetTransform()->GetPos(), zaxisend, glm::vec3(0, 0, 1));
	}

}

void EditorGizmos::SetTarget(GameObject * t)
{
	target = t;
}
void EditorGizmos::SwitchMode(int index)
{
	ModeIndex = index;
}
#endif