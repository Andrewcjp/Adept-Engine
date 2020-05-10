
#include "EditorCore.h"
#include "EditorWindow.h"
#include "Core/Assets/Scene.h"
#include "UI/UIManager.h"
#include "Inspector.h"
#if WITH_EDITOR
EditorCore::EditorCore()
{}


EditorCore::~EditorCore()
{}

void EditorCore::SetSelectedObjectIndex(int index)
{
	SelectedObject = BaseWindow::GetScene()->GetObjects()[index];
}

GameObject * EditorCore::GetSelectedObject()
{
	return SelectedObject;
}

void EditorCore::SetSelectedObject(GameObject * newobj)
{
	SelectedObject = newobj;
	UIManager::Get()->GetInspector()->SetSelectedObject(GetSelectedObject());
}
#endif