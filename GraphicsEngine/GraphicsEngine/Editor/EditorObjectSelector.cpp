#include "stdafx.h"
#include "EditorObjectSelector.h"
#include "../Core/GameObject.h"
#include "../Core/Engine.h"
#include "../Physics/PhysxEngine.h"
#include "EditorWindow.h"
#include "../Rendering/Core/DebugLineDrawer.h"
#include "../Rendering/Core/Camera.h"
EditorObjectSelector::EditorObjectSelector()
{
	pengine = (PhysxEngine*)Engine::PhysEngine;
}


EditorObjectSelector::~EditorObjectSelector()
{
}

void EditorObjectSelector::init()
{

}
void EditorObjectSelector::LinkPhysxBodysToGameObjects(std::vector<GameObject*>& objects)
{
	for (int i = 0; i < objects.size(); i++)
	{
		pengine->AddBoxCollisionToEditor(objects[i]);
	}
}
GameObject * EditorObjectSelector::RayCastScene(int x, int y, Camera* cam, std::vector<GameObject*>& objects)
{
	//campos;
	PxRaycastBuffer hit;
	/*campos.x += (x - (2 / EditorWindow::GetWidth())) / 1000;
	campos.y += (y - (2 / EditorWindow::GetHeight())) / 1000;*/
	
	glm::vec3 camforward = glm::vec4(cam->GetForward(), 1.0f);// *cam->GetProjection();
	DebugLineDrawer::instance->AddLine(cam->GetPosition(), cam->GetPosition() + camforward * 1000, glm::vec3(1, 0, 0), 10);
	if (pengine->RayCastEditorScene(cam->GetPosition(), glm::normalize(camforward), 50, &hit))
	{
		for (int i = 0; i < objects.size(); i++)
		{
			if (objects[i]->SelectionShape == hit.block.actor)
			{
				return objects[i];
			}
		}
	}
	return nullptr;
}
