#include "EditorGizmos.h"
#include "Core/GameObject.h"

#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Input.h"
EditorGizmos::EditorGizmos()
{
	HandleLength = 5;
}


EditorGizmos::~EditorGizmos()
{
}
void EditorGizmos::UpdateAxis(float amt, Axis axis)
{
	glm::vec3 newpos = target->GetTransform()->GetPos();
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
				UpdateAxis((float)(XStartPos - Input::GetMouseInputAsAxis().x)*Scale);
			}
			else
			{
				UpdateAxis((float)(YStartPos - Input::GetMouseInputAsAxis().y)*Scale);
			}
			
		}
		YStartPos = (int)Input::GetMouseInputAsAxis().y;
		XStartPos = (int)Input::GetMouseInputAsAxis().x;
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
