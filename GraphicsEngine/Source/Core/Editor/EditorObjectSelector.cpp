
#include "EditorObjectSelector.h"
#include "Core/GameObject.h"
#include "Core/Engine.h"
#include "Physics/PhysicsEngine.h"
#include "EditorWindow.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Rendering/Core/Camera.h"
#include "Core/EngineTypes.h"
EditorObjectSelector::EditorObjectSelector()
{
	pengine = Engine::PhysEngine;
}


EditorObjectSelector::~EditorObjectSelector()
{}

void EditorObjectSelector::init()
{

}
void EditorObjectSelector::LinkPhysxBodysToGameObjects(const std::vector<GameObject*>& objects)
{
	for (int i = 0; i < objects.size(); i++)
	{
		//pengine->AddBoxCollisionToEditor(objects[i]);
	}
}
GameObject * EditorObjectSelector::RayCastScene(int x, int y, Camera* cam, const std::vector<GameObject*>& objects)
{

	/*physx::PxRaycastBuffer hit;*/
	glm::vec3 camforward = glm::vec4(cam->GetForward(), 1.0f);// *cam->GetProjection();
	glm::vec3 origin;
	glm::vec3 dir = -cam->GetForward();

	cam->GetRayAtScreenPos(((float)x), ((float)y), dir, origin);
	//cam->GetRayAtScreenPos(((float)x / (float)EditorWindow::GetWidth()), ((float)y / (float)EditorWindow::GetHeight()), dir, origin);
	//origin = /*cam->GetPosition() +*/ cam->ScreenPointToWorld(x, y);
	if (DebugLineDrawer::Get() != nullptr)
	{
		//		DebugLineDrawer::instance->AddLine(origin, origin + glm::normalize(-dir) * 1000, Colours::RED, 10);
	}
	/*if (pengine->RayCastEditorScene(origin, glm::normalize(-dir), 15000, &hit))
	{
		for (int i = 0; i < objects.size(); i++)
		{
			if (objects[i]->SelectionShape == hit.block.actor)
			{
				return objects[i];
			}
		}
	}*/
	return nullptr;
}
